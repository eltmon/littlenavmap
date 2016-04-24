/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
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

#ifndef ROUTENETWORKBASE_H
#define ROUTENETWORKBASE_H

#include <QHash>
#include <QVector>

#include <common/maptypes.h>

class QSqlRecord;

namespace  atools {
namespace sql {
class SqlDatabase;
class SqlQuery;
}
namespace geo {
class Pos;
class Rect;
}
}

namespace nw {

enum Mode
{
  ROUTE_NONE = 0x00,
  ROUTE_VOR = 0x01,
  ROUTE_VORDME = 0x02,
  ROUTE_DME = 0x04,
  ROUTE_NDB = 0x08,
  ROUTE_VICTOR = 0x10,
  ROUTE_JET = 0x20
};

Q_DECLARE_FLAGS(Modes, Mode);
Q_DECLARE_OPERATORS_FOR_FLAGS(nw::Modes);

enum Type
{
  VOR = 0,
  VORDME = 1,
  DME = 2,
  NDB = 3,
  WAYPOINT_VICTOR = 4,
  WAYPOINT_JET = 5,
  WAYPOINT_BOTH = 6,
  START = 10,
  DESTINATION = 11,
  NONE = 20
};

struct Edge
{
  Edge()
  {
  }

  Edge(int to, int distance = 0, int minimumAltitude = -1, nw::Type edgeType = nw::NONE)
    : toNodeId(to), distanceMeter(distance), minAltFt(minimumAltitude), type(edgeType)
  {

  }

  int toNodeId, distanceMeter, minAltFt, airwayId;
  nw::Type type;

  bool operator==(const nw::Edge& other) const
  {
    return toNodeId == other.toNodeId;
  }

  bool operator!=(const nw::Edge& other) const
  {
    return !operator==(other);
  }

};

inline int qHash(const nw::Edge& edge)
{
  return edge.toNodeId;
}

struct Node
{
  Node()
  {
  }

  Node(int nodeId, nw::Type nodeType, const atools::geo::Pos& position, int nodeRange = 0)
    : id(nodeId), range(nodeRange), pos(position), type(nodeType)
  {

  }

  int id = -1;
  int range;
  QVector<Edge> edges;
  atools::geo::Pos pos;
  nw::Type type;

  bool operator==(const nw::Node& other) const
  {
    return id == other.id && type == other.type;
  }

  bool operator!=(const nw::Node& other) const
  {
    return !operator==(other);
  }

};

inline int qHash(const nw::Node& node)
{
  return node.id;
}

}

Q_DECLARE_TYPEINFO(nw::Node, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(nw::Edge, Q_PRIMITIVE_TYPE);

class RouteNetworkBase
{
public:
  RouteNetworkBase(atools::sql::SqlDatabase *sqlDb, const QString& nodeTableName,
                   const QString& edgeTableName, const QStringList& nodeExtraColumns,
                   const QStringList& edgeExtraColumns);
  virtual ~RouteNetworkBase();

  void setMode(nw::Modes mode);

  nw::Node getNodeById(int id)
  {
    return fetchNode(id);
  }

  nw::Node getNodeByNavId(int id, nw::Type type);
  void getNavIdAndTypeForNode(int nodeId, int& navId, nw::Type& type);

  void initQueries();
  void deInitQueries();
  nw::Node fetchNode(int id);
  nw::Node fetchNode(float lonx, float laty, bool loadSuccessors, int id);

  atools::sql::SqlQuery *nodeByNavIdQuery = nullptr, *nodeNavIdAndTypeQuery = nullptr,
  *nearestNodesQuery = nullptr, *nodeByIdQuery = nullptr, *edgeToQuery = nullptr,
  *edgeFromQuery = nullptr;

  void getNeighbours(const nw::Node& from, QVector<nw::Node>& neighbours, QVector<int>* distances, QVector<int>* airwayIds);
  void addStartAndDestinationNodes(const atools::geo::Pos& from, const atools::geo::Pos& to);

  void clear();

  nw::Node getStartNode() const;
  nw::Node getDestinationNode() const;

  int getNumberOfNodesDatabase();
  int getNumberOfNodesCache() const;

protected:
  void addDestNodeEdges(nw::Node& node);
  void cleanDestNodeEdges();

  void bindCoordRect(const atools::geo::Rect& rect, atools::sql::SqlQuery *query);
  bool checkType(nw::Type type);
  nw::Node createNode(const QSqlRecord& rec);
  nw::Edge createEdge(const QSqlRecord& rec, int toNodeId);

  const int START_NODE_ID = -10;
  const int DESTINATION_NODE_ID = -20;
  int numNodesDb = -1;

  atools::geo::Rect startNodeRect, destinationNodeRect;
  atools::geo::Pos startPos, destinationPos;
  QSet<int> destinationNodePredecessors;

  atools::sql::SqlDatabase *db;
  nw::Modes mode;
  QHash<int, nw::Node> nodes;

  QString nodeTable, edgeTable;
  QStringList nodeExtraCols, edgeExtraCols;
};

#endif // ROUTENETWORKBASE_H