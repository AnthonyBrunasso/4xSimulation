#include "ai_evaluations.h"

#include "player.h"
#include "city.h"
#include "ai_barbarians.h"
#include "tile.h"
#include "search.h"
#include "world_map.h"
#include "units.h"
#include "hex.h"
#include "improvements.h"

#include <iostream>

float NeedsColonize::operator()(uint32_t player_id, float threshold) {
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


float NeedsProduce::operator()(uint32_t player_id, float threshold) {
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

float HasUnits::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Has units evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating its unit state." << std::endl;
  if (!player->m_units.empty()) {
    return threshold + 1.0f;
  }

  std::cout << player->m_name << " has no units." << std::endl;
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
    case AI_TYPE::BARBARIAN:
      return ::discovered_cities_barbarian(player, threshold);
    case AI_TYPE::HUMAN:
    default:
      std::cout << "DiscoveredCities evaluation not evaluated for: " << get_ai_name(player->m_ai_type) << std::endl;
      return NOOP_EVALUATION;
  }

  return NOOP_EVALUATION;
}

float UnitEvaluation::operator()(uint32_t player_id, float threshold) {
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player id. Attack evaluation. " << std::endl;
    return NOOP_EVALUATION;
  }

  AIState* state = p->m_ai_state;
  if (!state) {
    std::cout << p->m_name << " has no ai state. This is probably a bug." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << p->m_name << " issuing unit orders." << std::endl;

  // Loop over all the players units, check if if an enemy unit or city is in range.
  bool result = false;
  // Find the closest city or unit and create an order for it. 
  // If none are found in proximity create a wander order.
  auto check_proximity = [&result, &player_id, &state](Unit& owned_unit) {
    uint32_t range = owned_unit.m_combat_stats.m_range;
    uint32_t attack = owned_unit.m_combat_stats.m_attack;
    auto unit_check = [&result, &range, &owned_unit, &player_id, &state](const Unit& unit) {
      // Existence of a unit means this evaluation was successful.
      result = true;

      // Don't attack owned units.
      if (unit.m_owner_id == player_id) {
        return false;
      }
     
      AI_ORDER_TYPE order = AI_ORDER_TYPE::APPROACH_UNIT;
      // If the unit can be attacked attack it.
      if (hex::cube_distance(unit.m_location, owned_unit.m_location) <= range) {
        order = AI_ORDER_TYPE::ATTACK_UNIT;
      }
     
      // Otherwise just go towards it.
      state->add_order(owned_unit.m_unique_id, unit.m_unique_id, order);
      return true;
    };

    auto improvement_check = [&result, &range, &owned_unit, &player_id, &state](const Improvement& i) {
      result = true;

      if (i.m_owner_id == player_id) {
        return false;
      }

      // We need to move to an improvement before being able to pillage it.
      AI_ORDER_TYPE order = AI_ORDER_TYPE::MOVE;
      if (hex::cube_distance(owned_unit.m_location, i.m_location) == 0) {
        order = AI_ORDER_TYPE::PILLAGE_IMPROVEMENT;
      }

      state->add_order(owned_unit.m_unique_id, i.m_unique_id, order);
    };
   
    auto city_check = [&range, &owned_unit, &player_id, &state](const City& c) {
      // Don't care about owned cities.
      if (c.m_owner_id == player_id) {
        return false;
      }

      // Attacking cities not implemented yet. Approach it.
      state->add_order(owned_unit.m_unique_id, c.m_id, AI_ORDER_TYPE::APPROACH_CITY);
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
    state->add_order(owned_unit.m_unique_id, 0, AI_ORDER_TYPE::WANDER);
  };

  player::for_each_player_unit(player_id, check_proximity);
  if (result) {
    return threshold + 1.0f;
  }

  return threshold - 1.0f;
}
