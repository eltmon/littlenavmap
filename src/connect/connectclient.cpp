/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "connect/connectclient.h"

#include "common/constants.h"
#include "connect/connectdialog.h"
#include "fs/sc/simconnectreply.h"
#include "fs/sc/datareaderthread.h"
#include "gui/dialog.h"
#include "gui/errorhandler.h"
#include "gui/mainwindow.h"
#include "gui/widgetstate.h"

#include <QDataStream>
#include <QTcpSocket>
#include <QWidget>
#include <QApplication>
#include <QThread>

using atools::fs::sc::DataReaderThread;

ConnectClient::ConnectClient(MainWindow *parent)
  : QObject(parent), mainWindow(parent), metarIdentCache(WEATHER_TIMEOUT_FS_SECS)
{
  dialog = new ConnectDialog(mainWindow);

  if(DataReaderThread::isSimconnectAvailable())
  {
    dataReader = new DataReaderThread(mainWindow, false);
    dataReader->setReconnectRateSec(DIRECT_RECONNECT_SEC);

    connect(dataReader, &DataReaderThread::postSimConnectData, this,
            &ConnectClient::postSimConnectData);
    connect(dataReader, &DataReaderThread::postLogMessage, this,
            &ConnectClient::postLogMessage);

    connect(dataReader, &DataReaderThread::connectedToSimulator, this,
            &ConnectClient::connectedToSimulatorDirect);
    connect(dataReader, &DataReaderThread::disconnectedFromSimulator, this,
            &ConnectClient::disconnectedFromSimulatorDirect);
    connect(dialog, &ConnectDialog::directUpdateRateChanged, dataReader,
            &DataReaderThread::setUpdateRate);
  }

  connect(dialog, &ConnectDialog::disconnectClicked, this, &ConnectClient::disconnectClicked);
  connect(dialog, &ConnectDialog::autoConnectToggled, this, &ConnectClient::autoConnectToggled);

  reconnectNetworkTimer.setSingleShot(true);
  connect(&reconnectNetworkTimer, &QTimer::timeout, this, &ConnectClient::connectInternal);

  flushQueuedRequestsTimer.setInterval(2000);
  connect(&flushQueuedRequestsTimer, &QTimer::timeout, this, &ConnectClient::flushQueuedRequests);
  flushQueuedRequestsTimer.start();
}

ConnectClient::~ConnectClient()
{
  qDebug() << Q_FUNC_INFO;

  flushQueuedRequestsTimer.stop();
  reconnectNetworkTimer.stop();

  disconnectClicked();

  qDebug() << Q_FUNC_INFO << "delete dataReader";
  delete dataReader;

  qDebug() << Q_FUNC_INFO << "delete dialog";
  delete dialog;
}

void ConnectClient::flushQueuedRequests()
{
  if(!queuedRequests.isEmpty())
  {
    atools::fs::sc::WeatherRequest req = queuedRequests.takeLast();
    requestWeather(req.getStation(), req.getPosition());
  }
}

void ConnectClient::connectToServerDialog()
{
  dialog->setConnected(isConnected());

  // Show dialog
  int retval = dialog->exec();
  dialog->hide();

  if(retval == QDialog::Accepted)
  {
    silent = false;
    closeSocket(false);

    if(dataReader != nullptr)
    {
      dataReader->setTerminate(true);
      dataReader->wait();
      dataReader->setTerminate(false);
    }

    connectInternal();
  }
}

void ConnectClient::tryConnectOnStartup()
{
  if(dialog->isAutoConnect())
  {
    reconnectNetworkTimer.stop();

    // Do not show an error dialog
    silent = true;
    connectInternal();
  }
}

void ConnectClient::connectedToSimulatorDirect()
{
  mainWindow->setConnectionStatusMessageText(tr("Connected"),
                                             tr("Connected to local flight simulator."));
  dialog->setConnected(isConnected());
  emit connectedToSimulator();
  emit weatherUpdated();
}

void ConnectClient::disconnectedFromSimulatorDirect()
{
  // Try to reconnect if it was not unlinked by using the disconnect button
  if(dialog->isAutoConnect() && dialog->isConnectDirect() && !manualDisconnect)
    connectInternal();
  else
    mainWindow->setConnectionStatusMessageText(tr("Disconnected"),
                                               tr("Disconnected from local flight simulator."));
  dialog->setConnected(isConnected());

  metarIdentCache.clear();
  outstandingReplies.clear();
  queuedRequests.clear();

  emit disconnectedFromSimulator();
  emit weatherUpdated();

  manualDisconnect = false;
}

/* Posts data received directly from simconnect or the socket and caches any metar reports */
void ConnectClient::postSimConnectData(atools::fs::sc::SimConnectData dataPacket)
{
  emit dataPacketReceived(dataPacket);

  if(!dataPacket.getMetars().isEmpty())
  {
    if(verbose)
      qDebug() << "Metars number" << dataPacket.getMetars().size();

    for(const atools::fs::sc::MetarResult& metar : dataPacket.getMetars())
    {
      QString ident = metar.requestIdent;
      if(verbose)
      {
        qDebug() << "ConnectClient::postSimConnectData metar ident to cache ident"
                 << ident << "pos" << metar.requestPos.toString();
        qDebug() << "Station" << metar.metarForStation;
        qDebug() << "Nearest" << metar.metarForNearest;
        qDebug() << "Interpolated" << metar.metarForInterpolated;
      }

      metarIdentCache.insert(ident, atools::fs::sc::MetarResult(metar));
    }

    emit weatherUpdated();
  }
}

void ConnectClient::postLogMessage(QString message, bool warning)
{
  Q_UNUSED(message);
  Q_UNUSED(warning);

  // mainWindow->setStatusMessage(message);
}

void ConnectClient::saveState()
{
  dialog->saveState();
}

void ConnectClient::restoreState()
{
  dialog->restoreState();

  if(dataReader != nullptr)
    dataReader->setUpdateRate(dialog->getDirectUpdateRateMs());
}

atools::fs::sc::MetarResult ConnectClient::requestWeather(const QString& station,
                                                          const atools::geo::Pos& pos)
{
  static atools::fs::sc::MetarResult EMPTY;

  // Ignore cache if not connected
  if(!isConnected())
    return EMPTY;

  const atools::fs::sc::MetarResult *result = metarIdentCache.value(station);
  if(result != nullptr)
    return atools::fs::sc::MetarResult(*result);
  else
  {
    if(verbose)
      qDebug() << "ConnectClient::requestWeather" << station;

    if((socket != nullptr && socket->isOpen()) || (dataReader != nullptr && dataReader->isConnected()))
    {
      atools::fs::sc::WeatherRequest weatherRequest;
      weatherRequest.setStation(station);
      weatherRequest.setPosition(pos);

      if(outstandingReplies.isEmpty())
        // Nothing wating for reply - request now
        requestWeather(weatherRequest);
      else if(!outstandingReplies.contains(weatherRequest.getStation()))
        // Waiting reply - queue request
        queuedRequests.append(weatherRequest);

      if(verbose)
        qDebug() << "=== queuedRequests" << queuedRequests.size();
    }
    return EMPTY;
  }
}

void ConnectClient::requestWeather(const atools::fs::sc::WeatherRequest& weatherRequest)
{
  if(dataReader != nullptr && dataReader->isConnected())
    dataReader->setWeatherRequest(weatherRequest);

  if(socket != nullptr && socket->isOpen() && outstandingReplies.isEmpty())
  {
    if(verbose)
      qDebug() << "requestWeather" << weatherRequest.getStation();
    atools::fs::sc::SimConnectReply reply;
    reply.setCommand(atools::fs::sc::CMD_WEATHER_REQUEST);
    reply.setWeatherRequest(weatherRequest);
    writeReplyToSocket(reply);
    outstandingReplies.insert(weatherRequest.getStation());
  }
}

void ConnectClient::autoConnectToggled(bool state)
{
  if(state == false)
  {
    reconnectNetworkTimer.stop();

    if(dataReader != nullptr)
    {
      if(dataReader->isReconnecting())
      {
        qDebug() << "Stopping reconnect";
        dataReader->setTerminate(true);
        dataReader->wait();
        dataReader->setTerminate(false);
        qDebug() << "Stopping reconnect done";
      }
    }
  }
}

/* Called by signal ConnectDialog::disconnectClicked */
void ConnectClient::disconnectClicked()
{
  qDebug() << Q_FUNC_INFO;

  reconnectNetworkTimer.stop();

  if(dataReader != nullptr)
    if(dataReader->isConnected())
      // Tell disconnectedFromSimulatorDirect not to reconnect
      manualDisconnect = true;

  if(dataReader != nullptr)
  {
    dataReader->setTerminate(true);
    dataReader->wait();
    dataReader->setTerminate(false);
  }

  // Close but do not allow reconnect if auto is on
  closeSocket(false);
}

void ConnectClient::connectInternal()
{
  if(dialog->isConnectDirect())
  {
    qDebug() << "Starting direct connection";
    // Datareader has its own reconnect mechanism
    if(dataReader != nullptr)
      dataReader->start();

    mainWindow->setConnectionStatusMessageText(tr("Connecting..."),
                                               tr("Trying to connect to local flight simulator."));
  }
  else if(socket == nullptr)
  {
    // qDebug() << "Starting network connection";

    // Create new socket and connect signals
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &ConnectClient::readFromSocket);
    connect(socket, &QTcpSocket::connected, this, &ConnectClient::connectedToServerSocket);
    connect(socket,
            static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, &ConnectClient::readFromSocketError);

    qDebug() << "Connecting to" << dialog->getHostname() << ":" << dialog->getPort();
    socket->connectToHost(dialog->getHostname(), dialog->getPort(), QAbstractSocket::ReadWrite);

    mainWindow->setConnectionStatusMessageText(tr("Connecting..."),
                                               tr("Trying to connect to remote flight simulator on \"%1\".").
                                               arg(dialog->getHostname()));
  }
}

bool ConnectClient::isConnected() const
{
  if(dataReader != nullptr)
    return (socket != nullptr && socket->isOpen()) || dataReader->isConnected();
  else
    return socket != nullptr && socket->isOpen();
}

/* Called by signal QAbstractSocket::error */
void ConnectClient::readFromSocketError(QAbstractSocket::SocketError error)
{
  Q_UNUSED(error);
  // qDebug() << Q_FUNC_INFO << error;

  reconnectNetworkTimer.stop();

  qWarning() << "Error reading from" << socket->peerName() << ":" << dialog->getPort()
             << socket->errorString() << "open" << socket->isOpen() << "state" << socket->state();

  if(!silent)
  {
    if(socket->error() == QAbstractSocket::RemoteHostClosedError)
    {
      // Nicely closed on the other end
      atools::gui::Dialog(mainWindow).showInfoMsgBox(lnm::ACTIONS_SHOWDISCONNECTINFO,
                                                     tr("Little Navconnect closed connection."),
                                                     tr("Do not &show this dialog again."));
    }
    else
    {
      QString msg = tr("Error in server connection: %1 (%2).%3").
                    arg(socket->errorString()).
                    arg(socket->error()).
                    arg(dialog->isAutoConnect() ? tr("\nWill retry to connect.") : QString());

      // Closed due to error
      QMessageBox::critical(mainWindow, QApplication::applicationName(), msg,
                            QMessageBox::Close, QMessageBox::NoButton);
    }
  }

  // Close and allow restart if auto is on
  closeSocket(true);
}

void ConnectClient::closeSocket(bool allowRestart)
{
  // qDebug() << Q_FUNC_INFO;

  QAbstractSocket::SocketError error = QAbstractSocket::SocketError::UnknownSocketError;
  QString peer("Unknown"), errorStr("No error");
  if(socket != nullptr)
  {
    error = socket->error();
    errorStr = socket->errorString();
    peer = socket->peerName();

    socket->abort();
    socket->deleteLater();
    socket = nullptr;
  }

  delete simConnectData;
  simConnectData = nullptr;

  QString msgTooltip, msg;
  if(error == QAbstractSocket::RemoteHostClosedError || error == QAbstractSocket::UnknownSocketError)
  {
    msg = tr("Disconnected");
    msgTooltip = tr("Disconnected from remote flight simulator on \"%1\".").arg(peer);
  }
  else
  {
    if(silent)
    {
      msg = tr("Connecting...");
      msgTooltip = tr("Error while trying to connect to \"%1\": %2 (%3).\nWill retry.").
                   arg(peer).arg(errorStr).arg(error);
    }
    else
    {
      msg = tr("Connect Error");
      msgTooltip = tr("Error in server connection to \"%1\": %2 (%3)").arg(peer).arg(errorStr).arg(error);
    }
  }

  mainWindow->setConnectionStatusMessageText(msg, msgTooltip);
  dialog->setConnected(isConnected());

  metarIdentCache.clear();
  outstandingReplies.clear();
  queuedRequests.clear();

  if(socketConnected)
  {
    // qDebug() << Q_FUNC_INFO << "***** sending messages";
    emit disconnectedFromSimulator();
    emit weatherUpdated();
    socketConnected = false;
  }

  if(!dialog->isConnectDirect() && dialog->isAutoConnect() && allowRestart)
  {
    // For socket based connection use a timer - direct connection reconnects automatically
    silent = true;
    reconnectNetworkTimer.start(SOCKET_RECONNECT_SEC * 1000);
  }
  else
    silent = false;
}

void ConnectClient::writeReplyToSocket(atools::fs::sc::SimConnectReply& reply)
{
  reply.write(socket);

  if(reply.getStatus() != atools::fs::sc::OK)
  {
    // Something went wrong - shutdown
    QMessageBox::critical(mainWindow, QApplication::applicationName(),
                          QString(tr("Error writing reply to Little Navconnect: %1.")).
                          arg(reply.getStatusText()));
    closeSocket(false);
    return;
  }

  if(!socket->flush())
    qWarning() << "Reply to server not flushed";
}

/* Called by signal QTcpSocket::connected */
void ConnectClient::connectedToServerSocket()
{
  qInfo() << Q_FUNC_INFO << "Connected to" << socket->peerName() << ":" << socket->peerPort();
  socketConnected = true;
  reconnectNetworkTimer.stop();

  mainWindow->setConnectionStatusMessageText(tr("Connected"),
                                             tr("Connected to remote flight simulator on \"%1\".").
                                             arg(socket->peerName()));

  silent = false;

  dialog->setConnected(isConnected());

  // Let other program parts know about the new connection
  emit connectedToSimulator();
  emit weatherUpdated();
}

/* Called by signal QTcpSocket::readyRead - read data from socket */
void ConnectClient::readFromSocket()
{
  while(socket->bytesAvailable())
  {
    if(verbose)
      qDebug() << "readFromSocket" << socket->bytesAvailable();
    if(simConnectData == nullptr)
      // Need to keep the data in background since this method can be called multiple times until the data is filled
      simConnectData = new atools::fs::sc::SimConnectData;

    bool read = simConnectData->read(socket);
    if(simConnectData->getStatus() != atools::fs::sc::OK)
    {
      // Something went wrong - shutdown
      QMessageBox::critical(mainWindow, QApplication::applicationName(),
                            QString(tr("Error reading data from Little Navconnect: %1.")).
                            arg(simConnectData->getStatusText()));
      closeSocket(false);
      return;
    }

    if(verbose)
      qDebug() << "readFromSocket 2" << socket->bytesAvailable();
    if(read)
    {
      if(verbose)
        qDebug() << "readFromSocket id " << simConnectData->getPacketId();

      if(simConnectData->getPacketId() > 0)
      {
        // Data was read completely and successfully - reply to server
        atools::fs::sc::SimConnectReply reply;
        reply.setPacketId(simConnectData->getPacketId());
        writeReplyToSocket(reply);
      }
      else if(!simConnectData->getMetars().isEmpty())
      {
        for(const atools::fs::sc::MetarResult& metar : simConnectData->getMetars())
          outstandingReplies.remove(metar.requestIdent);

        if(outstandingReplies.isEmpty() && !queuedRequests.isEmpty())
          requestWeather(queuedRequests.takeLast());
      }

      // Send around in the application
      postSimConnectData(*simConnectData);
      delete simConnectData;
      simConnectData = nullptr;
    }
    else
      return;
  }
  if(verbose)
    qDebug() << "outstanding" << outstandingReplies;
}
