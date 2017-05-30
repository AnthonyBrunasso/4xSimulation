#include "ai_empire_evaluations.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Vector3.hpp"
#include "ai_state.h"
#include "city.h"
#include "combat.h"

#include "hex.h"
#include "improvement.h"
#include "player.h"
#include "search.h"
#include "step_generated.h"
#include "unit.h"
#include "world_map.h"

float EmpireColonize::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Needs colonization evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating colonization state." << std::endl;
  if (player->m_cities.empty()) {
    std::cout << player->m_name << " needs to colonize." << std::endl;
    return threshold + 1.0f;
  }
  else {
    return threshold - 1.0f;
  }
}


float EmpireProduce::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Needs production evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating production state." << std::endl;
  for (auto id : player->m_cities) {
    City* city = city::get_city(id);
    if (!city) continue;
    // If the cities production queue is empty the player needs production.
    if (!city->IsConstructing()) {
      std::cout << player->m_name << " needs production." << std::endl;
      return threshold + 1.0f;
    }
  }

  // Player doesn't have an available city for production.
  return threshold - 1.0f;
}

float discovered_cities_barbarian(Player* player, float threshold) {
  std::cout << player->m_name << " evaluating nearby cities." << std::endl;
  if (player->m_discovered_cities.empty()) {
    return threshold - 1.0f;
  }
  return threshold + 1.0f;
}

float DiscoveredCities::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Discovered city evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  switch (player->m_ai_type) {
    case fbs::AI_TYPE::BARBARIAN:
      return ::discovered_cities_barbarian(player, threshold);
    case fbs::AI_TYPE::HUMAN:
    default:
      std::cout << "DiscoveredCities evaluation not evaluated for: " << fbs::EnumNameAI_TYPE(player->m_ai_type) << std::endl;
      return NOOP_EVALUATION;
  }

  return NOOP_EVALUATION;
}

float EmpireUnitOrder::operator()(uint32_t player_id, float threshold) {
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player id. Attack evaluation. " << std::endl;
    return NOOP_EVALUATION;
  }

  std::shared_ptr<AIState> state = p->m_ai_state;
  if (!state) {
    std::cout << p->m_name << " has no ai state. This is probably a bug." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << p->m_name << " issuing unit orders." << std::endl;

  // Loop over all the players units, check if an enemy unit or city is in range.
  bool result = false;
  // Find the closest city or unit and create an order for it. 
  // If none are found in proximity create a wander order.
  auto check_proximity = [&result, &player_id, &state](Unit& owned_unit) {
    float range = owned_unit.m_combat_stats.m_range;
    float attack = owned_unit.m_combat_stats.m_attack;
    auto unit_check = [&result, &range, &owned_unit, &player_id, &state](const Unit& unit) {
      // Existence of a unit means this evaluation was successful.
      result = true;

      // Don't attack owned units.
      if (unit.m_owner_id == player_id) {
        return false;
      }
     
      fbs::AI_ORDER_TYPE order = fbs::AI_ORDER_TYPE::APPROACH_UNIT;
      // If the unit can be attacked attack it.
      if (hex::cube_distance(unit.m_location, owned_unit.m_location) <= range) {
        order = fbs::AI_ORDER_TYPE::ATTACK_UNIT;
      }
     
      // Otherwise just go towards it.
      state->add_order(owned_unit.m_id, unit.m_id, order);
      return true;
    };

    auto improvement_check = [&result, &range, &owned_unit, &player_id, &state](const Improvement& i) {
      result = true;

      if (i.m_owner_id == player_id) {
        return false;
      }

      // We need to move to an improvement before being able to pillage it.
      fbs::AI_ORDER_TYPE order = fbs::AI_ORDER_TYPE::APPROACH_IMPROVEMENT;
      if (hex::cube_distance(owned_unit.m_location, i.m_location) == 0) {
        order = fbs::AI_ORDER_TYPE::PILLAGE_IMPROVEMENT;
      }

      state->add_order(owned_unit.m_id, i.m_id, order);
      return true;
    };
   
    auto city_check = [&range, &owned_unit, &player_id, &state](const City& c) {
      // Don't care about owned cities.
      if (c.m_owner_id == player_id) {
        return false;
      }

      // Attacking cities not implemented yet. Approach it.
      state->add_order(owned_unit.m_id, c.m_id, fbs::AI_ORDER_TYPE::APPROACH_CITY);
      return true;
    };
    // Approach or attack the found unit up to 4 range.
    if (attack && search::bfs_units(owned_unit.m_location, 4, world_map::get_map(), unit_check)) {
      return;
    }

    if (attack && search::bfs_improvements(owned_unit.m_location, 4, world_map::get_map(), improvement_check)) {
      return;
    }

    // Approch or attack a city up to 5 range if it has been discovered.
    if (attack && search::bfs_cities(owned_unit.m_location, 5, world_map::get_map(), city_check)) {
      return;
    }

    // Otherwise wander.
    state->add_order(owned_unit.m_id, 0, fbs::AI_ORDER_TYPE::WANDER);
  };

  player::for_each_player_unit(player_id, check_proximity);
  if (result) {
    return threshold + 1.0f;
  }

  return threshold - 1.0f;
}


float IdleMilitaryUnits::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Has units evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating its unit state." << std::endl;
  for (const auto& unit_id : player->m_units) {
    Unit* u = unit::get_unit(unit_id);
    if (!u) continue;
    if (u->m_action_points > 0) {
      m_state->m_idle_units.push_back(unit_id);
    }
  }

  if (m_state->m_idle_units.size() > 0)
  {
    return threshold + 1.0f;
  }

  std::cout << player->m_name << " has no idle units." << std::endl;
  return threshold - 1.0f;
}

float DefenderVsIdle::operator()(uint32_t player_id, float threshold) {
  std::cout << "DefenderVsIdle" << std::endl;
  Player* player = player::get_player(player_id);
  if (!player) return NOOP_EVALUATION;

  if (m_state->m_idle_units.size() <= player->m_cities.size()) {
    return threshold - 1.0f;
  }
 
  for (int i = 0; i < player->m_cities.size(); ++i) {
    m_state->m_idle_units.pop_back();
  }

  return threshold + 1.0f;
}

float Fortified::operator()(uint32_t player_id, float threshold) {
  // TODO: add fortify option to units
  std::cout << "Fortified" << std::endl;
  return threshold - 1.0f;
}

float ThreatenedVsAvailable::operator()(uint32_t player_id, float threshold) {
  std::cout << "ThreatenedVsAvailable" << std::endl;
  std::vector<uint32_t> threatened;
  float safe = threshold +1.f;
  float under_threat = threshold - 1.f;

  Player* player = player::get_player(player_id);
  if (!player) return safe; 
  
  if (player->m_omniscient) {
    unit::for_each_unit( [this, player_id] (const Unit& u) {
      if (u.m_owner_id != player_id) {
        m_state->m_threats.push_back(u.m_id);
      }
    });
    return under_threat;
  }

  for (uint32_t unit_id : m_state->m_idle_units) {
    Unit* u = unit::get_unit(unit_id);
    if (!u) continue;
    search::bfs_units(u->m_location, 2, world_map::get_map(), [&threatened, u, this](const Unit& other) {
      if (u->m_owner_id != other.m_owner_id) {
        threatened.push_back(u->m_id);
        m_state->m_threats.push_back(other.m_id);
        return true;
      }
      return false;
    });
  }

  if (threatened.size()) {
    m_state->m_idle_units.swap(threatened);
    return under_threat;
  }

  return safe;
}

float ApproachVsAttack::operator()(uint32_t player_id, float threshold) {
  std::cout << "ApproachVsAttack" << std::endl;
  if (m_state->m_idle_units.empty()) return NOOP_EVALUATION;
  if (m_state->m_threats.empty()) return NOOP_EVALUATION;

  uint32_t unit_id = m_state->m_idle_units.back();
  uint32_t threat_id = m_state->m_threats.back();
  Unit* unit = unit::get_unit(unit_id);
  Unit* threat = unit::get_unit(threat_id);

  m_state->m_target_location = threat->m_location;
  uint32_t distance = hex::cube_distance(unit->m_location, threat->m_location);
  if (distance > unit->m_combat_stats.m_range) {
    return threshold - 1.0f;
  }

  return threshold + 1.0f;
}

float FindCityVsGotoCity::operator()(uint32_t player_id, float threshold) {
  std::cout << "FindCityVsGotoCity" << std::endl;
  Player* player = player::get_player(player_id);
  if (!player) return NOOP_EVALUATION;

  if (player->m_discovered_cities.size() == 0) {
    return threshold - 1.f;
  }

  m_state->m_target_city = *player->m_discovered_cities.begin();

  return threshold + 1.0f;
}

float PillageVsSiege::operator()(uint32_t player_id, float threshold) {
  std::cout << "PillageVsSiege" << std::endl;
  Player* player = player::get_player(player_id);
  if (!player) return NOOP_EVALUATION;

  City* c = city::get_city(m_state->m_target_city);
  if (!c) return NOOP_EVALUATION;

  search::bfs_improvements(c->m_location, 3, world_map::get_map(), [player_id, this](const Improvement& imp) {
    if (imp.m_owner_id == player_id) return true;

    m_state->m_pillage_targets.push_back(imp.m_id);

    return true;
  });

  if (m_state->m_pillage_targets.size()) {
    return threshold - 1.f;
  }

  return threshold + 1.0f;
}

float ApproachVsSiege::operator()(uint32_t player_id, float threshold) {
  std::cout << "ApproachVsSiege" << std::endl;
  Player* player = player::get_player(player_id);
  if (!player) return NOOP_EVALUATION;

  uint32_t unit_id = m_state->m_idle_units.back();
  Unit* u = unit::get_unit(unit_id);
  City* c = city::get_city(m_state->m_target_city);

  if (!u) return NOOP_EVALUATION;
  if (!c) return NOOP_EVALUATION;

  m_state->m_target_location = c->m_location;
  if (hex::cube_distance(u->m_location, c->m_location) > u->m_combat_stats.m_range) {
    return threshold - 1.f;
  }

  return threshold + 1.0f;
}

float ApproachVsPillage::operator()(uint32_t player_id, float threshold) {
  std::cout << "ApproachVsPillage" << std::endl;
  Improvement* imp = improvement::get_improvement(m_state->m_pillage_targets.back());
  if (!imp) return NOOP_EVALUATION;
  Unit* unit = unit::get_unit(m_state->m_idle_units.back());
  if (!unit) return NOOP_EVALUATION;

  m_state->m_target_location = imp->m_location;
  if (hex::cube_distance(imp->m_location, unit->m_location) > 0) {
    return threshold - 1.f;
  }

  return threshold + 1.0f;
}

namespace empire_evaluations {
  EmpireColonize s_colonize;
  EmpireProduce s_produce;
  IdleMilitaryUnits s_idle_military_units;
  DiscoveredCities s_cities;
  EmpireUnitOrder s_unit_order;
}

EmpireColonize& empire_evaluations::get_colonize() {
  return s_colonize;
}

EmpireProduce& empire_evaluations::get_produce() {
  return s_produce;
}

DiscoveredCities& empire_evaluations::get_discovered_cities() {
  return s_cities;
}

EmpireUnitOrder& empire_evaluations::get_unit_order() {
  return s_unit_order;
}

IdleMilitaryUnits& empire_evaluations::has_idle_military() {
  return s_idle_military_units;
}

DefenderVsIdle& empire_evaluations::defender_vs_idle() {
  static DefenderVsIdle s_defender_vs_idle;
  return s_defender_vs_idle;
}

Fortified& empire_evaluations::has_fortified() {
  static Fortified s_fortified;
  return s_fortified;
}

ThreatenedVsAvailable& empire_evaluations::threatened_vs_available() {
  static ThreatenedVsAvailable s_threatened_vs_available;
  return s_threatened_vs_available;
}

ApproachVsAttack& empire_evaluations::approach_vs_attack() {
  static ApproachVsAttack s_approach_vs_attack;
  return s_approach_vs_attack;
}

FindCityVsGotoCity& empire_evaluations::find_city_vs_goto_city() {
  static FindCityVsGotoCity s_find_city_vs_goto_city;
  return s_find_city_vs_goto_city;
}

PillageVsSiege& empire_evaluations::pillage_vs_siege() {
  static PillageVsSiege s_pillage_vs_siege;
  return s_pillage_vs_siege;
}

ApproachVsSiege& empire_evaluations::approach_vs_siege() {
  static ApproachVsSiege s_approach_vs_siege;
  return s_approach_vs_siege;
}

ApproachVsPillage& empire_evaluations::approach_vs_pillage() {
  static ApproachVsPillage s_approach_vs_pillage;
  return s_approach_vs_pillage;
}
