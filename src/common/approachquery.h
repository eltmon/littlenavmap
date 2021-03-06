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

#ifndef LITTLENAVMAP_APPROACHQUERY_H
#define LITTLENAVMAP_APPROACHQUERY_H

#include "geo/pos.h"
#include "common/maptypes.h"

#include <QCache>
#include <QApplication>
#include <functional>

namespace atools {
namespace sql {
class SqlDatabase;
class SqlQuery;
}
}

class MapQuery;

/* Loads and caches approaches and transitions. The corresponding approach is also loaded and cached if a
 * transition is loaded since legs depend on each other.*/
class ApproachQuery
{
  Q_DECLARE_TR_FUNCTIONS(ApproachQuery)

public:
  ApproachQuery(atools::sql::SqlDatabase *sqlDb, MapQuery *mapQueryParam);
  virtual ~ApproachQuery();

  const maptypes::MapApproachLeg *getApproachLeg(const maptypes::MapAirport& airport, int approachId, int legId);
  const maptypes::MapApproachLeg *getTransitionLeg(const maptypes::MapAirport& airport, int legId);

  /* Get approach only */
  const maptypes::MapApproachLegs *getApproachLegs(const maptypes::MapAirport& airport, int approachId);

  /* Get transition and its approach */
  const maptypes::MapApproachLegs *getTransitionLegs(const maptypes::MapAirport& airport, int transitionId);

  /* Create all queries */
  void initQueries();

  /* Delete all queries */
  void deInitQueries();

private:
  maptypes::MapApproachLeg buildTransitionLegEntry();
  maptypes::MapApproachLeg buildApproachLegEntry();
  void buildLegEntry(atools::sql::SqlQuery *query, maptypes::MapApproachLeg& leg);

  void postProcessLegs(const maptypes::MapAirport& airport, maptypes::MapApproachLegs& legs);
  void processLegs(maptypes::MapApproachLegs& legs);

  /* Fill the courese and heading to intercept legs after all other lines are calculated */
  void processCourseInterceptLegs(maptypes::MapApproachLegs& legs);

  /* Fill calculatedDistance and calculated course fields */
  void processLegsDistanceAndCourse(maptypes::MapApproachLegs& legs);

  /* Add an artificial (not in the database) runway leg if no connection to the end is given */
  void processFinalRunwayLegs(const maptypes::MapAirport& airport, maptypes::MapApproachLegs& legs);

  /* Assign magnetic variation from the navaids */
  void updateMagvar(maptypes::MapApproachLegs& legs);
  void updateBoundingRectangle(maptypes::MapApproachLegs& legs);

  maptypes::MapApproachLegs *buildApproachLegs(const maptypes::MapAirport& airport, int approachId);
  maptypes::MapApproachLegs *fetchApproachLegs(const maptypes::MapAirport& airport, int approachId);
  maptypes::MapApproachLegs *fetchTransitionLegs(const maptypes::MapAirport& airport, int approachId, int transitionId);

  atools::sql::SqlDatabase *db;
  atools::sql::SqlQuery *approachLegQuery = nullptr, *transitionLegQuery = nullptr,
  *transitionIdForLegQuery = nullptr, *approachIdForTransQuery = nullptr,
  *runwayEndIdQuery = nullptr, *transitionQuery = nullptr, *approachQuery = nullptr;

  /* approach ID and transition ID to full lists
   * The approach also has to be stored for transitions since the handover can modify approach legs (CI legs, etc.) */
  QCache<int, maptypes::MapApproachLegs> approachCache, transitionCache;

  /* maps leg ID to approach/transition ID and index in list */
  QHash<int, std::pair<int, int> > approachLegIndex, transitionLegIndex;

  MapQuery *mapQuery = nullptr;

  /* Use this value as an id base for the artifical runway legs. Add id of the predecessor to it to be able to find the
   * leg again */
  Q_DECL_CONSTEXPR static int RUNWAY_LEG_ID_BASE = 1000000000;

};

#endif // LITTLENAVMAP_APPROACHQUERY_H
