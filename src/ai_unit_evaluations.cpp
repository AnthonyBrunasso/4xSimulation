#include "ai_unit_evaluations.h"

#include <iostream>

#include "unit.h"
#include "search.h"
#include "world_map.h"

float UnitFindNear::operator()(uint32_t unit_id, float threshold) {
  Unit* u = unit::get_unit(unit_id);
  if (!u) {
    std::cout << "Could not find unit id: " << unit_id << std::endl;
    return NOOP_EVALUATION;
  }

  auto unit_near = [&u](const Unit& unit) {
    if (u->m_owner_id != unit.m_owner_id) return true;
    return false;
  };

  if (search::bfs_units(u->m_location, 4, world_map::get_map(), unit_near)) {
    return threshold + 1.0f;
  }
  return threshold - 1.0f;
}

namespace unit_evaluations {
  UnitFindNear s_findnear;
}

UnitFindNear& unit_evaluations::get_find_near() {
  return s_findnear;
}
