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

#ifndef LITTLENAVMAP_MAPCOLORS_H
#define LITTLENAVMAP_MAPCOLORS_H

#include <QColor>
#include <QPen>
#include <QStyle>

namespace maptypes {
struct MapAirport;

}

/* All colors and pens used on the map and profile display */
namespace mapcolors {

/* General text pens */
const QPen textBackgroundPen = QPen(QBrush(QColor(Qt::lightGray)), 1, Qt::SolidLine, Qt::FlatCap);
const QPen textPen = QPen(QBrush(QColor(0, 0, 0)), 1, Qt::SolidLine, Qt::FlatCap);

/* Airport diagram background */
const QColor airportDetailBackColor = QColor(Qt::white);

const QColor taxiwayNameColor = QColor(Qt::black);
const QColor taxiwayNameBackgroundColor = QColor(255, 255, 120);
const QBrush taxiwayClosedBrush = QBrush(QColor(255, 255, 0), Qt::BDiagPattern);

const QColor runwayOutlineColor = QColor(Qt::black);
const QColor runwayOffsetColor = QColor(Qt::white);
const QBrush runwayBlastpadBrush = QBrush(Qt::yellow, Qt::DiagCrossPattern);
const QBrush runwayOverrunBrush = QBrush(QColor(180, 180, 0), Qt::DiagCrossPattern);

const QColor parkingOutlineColor = QColor(80, 80, 80);
const QColor helipadOutlineColor = QColor(Qt::black);
const QColor helipadMedicalOutlineColor = QColor(200, 0, 0);
const QColor activeTowerColor = QColor(Qt::red);
const QColor activeTowerOutlineColor = QColor(Qt::black);
const QColor inactiveTowerColor = QColor(Qt::lightGray);
const QColor inactiveTowerOutlineColor = QColor(Qt::darkGray);

const QColor darkParkingTextColor = QColor(Qt::black);
const QColor brightParkingTextColor = QColor(Qt::white);

const QColor towerTextColor = QColor(Qt::black);
const QColor runwayDimsTextColor = QColor(Qt::black);
const QColor runwayTextBackgroundColor = QColor(255, 255, 255, 170);

/* Text background color */
const QColor textBoxColor = QColor(Qt::white);

/* Text background color for flight plan waypoints */
const QColor routeTextBoxColor = QColor(255, 255, 150);

const QColor airportSymbolFillColor = QColor(Qt::white);

const QPen markBackPen = QPen(QBrush(QColor(0, 0, 0)), 6, Qt::SolidLine, Qt::FlatCap);
const QPen markFillPen = QPen(QBrush(QColor(255, 255, 0)), 2, Qt::SolidLine, Qt::FlatCap);
const QPen magneticPolePen = QPen(QBrush(QColor(Qt::darkGreen)), 2, Qt::SolidLine, Qt::FlatCap);

const QPen aircraftBackPen = QPen(QBrush(QColor(Qt::black)), 7, Qt::SolidLine, Qt::RoundCap);
const QPen aircraftFillPen = QPen(QBrush(QColor(Qt::yellow)), 4, Qt::SolidLine, Qt::RoundCap);
const QPen aircraftGroundBackPen = QPen(QBrush(QColor(Qt::darkGray)), 7, Qt::SolidLine, Qt::RoundCap);
const QPen aircraftGroundFillPen = QPen(QBrush(QColor(Qt::yellow)), 4, Qt::SolidLine, Qt::RoundCap);

const QPen homeBackPen = QPen(QBrush(QColor(0, 0, 0)), 2, Qt::SolidLine, Qt::FlatCap);
const QColor homeFillColor = QColor(Qt::yellow);

/* Objects highlighted because of selection in search */
const QColor highlightBackColor = QColor(Qt::black);
const QColor highlightColor = QColor(Qt::yellow);
const QColor highlightColorFast = QColor(Qt::darkYellow);

const QColor highlightApproachColor = QColor(150, 150, 255);
const QColor highlightApproachColorFast = QColor(0, 0, 150);

/* Flight plan line colors */
const QColor routeOutlineColor = QColor(Qt::black);
const QColor routeDragColor = QColor(Qt::darkYellow);

const QColor routeApproachOutlineColor = QColor(Qt::black);
const QColor routeApproachColor = QColor(255, 150, 0);
const QColor routeApproachMissedColor = QColor(255, 150, 0);

const QColor routeHighlightBackColor = QColor(Qt::black);
const QColor routeHighlightColor = QColor(Qt::green);
const QColor routeHighlightColorFast = QColor(Qt::darkGreen);

/* Text along route and approach segments */
const QColor routeTextColor = QColor(0, 0, 0);
const QColor routeTextBackgroundColor = QColor(255, 255, 255, 180);

const QColor routeApproachMissedTextColor = QColor(90, 90, 90);
const QColor routeApproachTextColor = QColor(0, 0, 0);

const QColor routeApproachPointColor = QColor(90, 90, 90);
const QColor routeUserPointColor = QColor(Qt::darkYellow);
/* Point not found in database */
const QColor routeInvalidPointColor = QColor(Qt::red);

const QColor rangeRingColor = QColor(Qt::red);
const QColor rangeRingTextColor = QColor(Qt::black);

const QColor distanceColor = QColor(Qt::black);
const QColor distanceRhumbColor = QColor(80, 80, 80);

/* Radio navaid colors */
const QColor vorSymbolColor = QColor(Qt::darkBlue);
const QColor ndbSymbolColor = QColor(Qt::darkRed);
const QColor markerSymbolColor = QColor(Qt::darkMagenta);
const QColor ilsSymbolColor = QColor(Qt::darkGreen);
const QColor ilsTextColor = QColor(0, 30, 0);
const QColor waypointSymbolColor = QColor(200, 0, 200);

/* Airway colors */
const QColor airwayVictorColor = QColor(150, 150, 150);
const QColor airwayJetColor = QColor(100, 100, 100);
const QColor airwayBothColor = QColor(100, 100, 100);
const QColor airwayTextColor = QColor(80, 80, 80);

/* Elevation profile colors and pens */
const QColor profileSkyColor(QColor(204, 204, 255));
const QColor profileSkyDarkColor(QColor(20, 20, 90));
const QColor profileBackgroundColor(Qt::white);

const QColor profileBackgroundDarkColor(QColor(20, 20, 20));

const QPen profileWaypointLinePen(Qt::gray, 1, Qt::SolidLine);
const QColor profileLandColor(Qt::darkGreen);
const QColor profileLandOutlineColor(Qt::black);

const QPen profleElevationScalePen(Qt::gray, 1, Qt::SolidLine);
const QPen profileSafeAltLinePen(Qt::red, 4, Qt::SolidLine);
const QPen profileSafeAltLegLinePen(QColor(255, 100, 0), 3, Qt::SolidLine);

const QColor nextWaypointColor(QColor(255, 100, 255));
const QColor nextWaypointColorDark(QColor(150, 20, 150));

/* Get an icon for the start type (RUNWAY, HELIPAD or WATER) */
const QIcon& iconForStartType(const QString& type);

/* General icon for parking (RAMP_MIL_*, GATE_*, RAMP_GA_* or RAMP_CARGO */
const QIcon& iconForParkingType(const QString& type);

/* Color for runway, apron and taxiway surface */
const QColor& colorForSurface(const QString& surface);

/* Color for detailed parking symbol in airport diagram (RAMP_MIL_*, GATE_*, RAMP_GA_* or RAMP_CARGO */
const QColor& colorForParkingType(const QString& type);

/* Color for airport symbol */
const QColor& colorForAirport(const maptypes::MapAirport& ap);

/* Alternating row background color for search tables */
const QColor& alternatingRowColor(int row, bool isSort);

const QPen aircraftTrailPen(float size);

} // namespace mapcolors

#endif // LITTLENAVMAP_MAPCOLORS_H
