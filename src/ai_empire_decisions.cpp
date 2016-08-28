#include "ai_empire_decisions.h"

#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include "network_types.h"
#include "tile.h"
#include "player.h"
#include "city.h"
#include "ai_shared.h"
#include "Vector3.hpp"
#include "format.h"
#include "game_types.h"
#include "util.h"
#include "random.h"
#include "world_map.h"
#include "city.h"
#include "production.h"
#include "unique_id.h"
#include "search.h"
#include "hex.h"

const size_t BUFFER_LEN = 256;
char s_ai_buffer[BUFFER_LEN];

void EmpireSettle::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Unable to find player to settle." << std::endl;
  }

  std::unordered_map<sf::Vector3i, bool> invalid_homes;   
  player::for_each_player([&invalid_homes](Player& player) {
    for (auto id : player.m_cities) {
      City* city = city::get_city(id);
      if (!city) continue;
      invalid_homes[city->m_location] = true;
    }
  });

  int start = 7;
  // Find a home.
  sf::Vector3i new_home = game_random::cube_coord(start);
  Tile* tile = world_map::get_tile(new_home);
  while (invalid_homes.find(new_home) != invalid_homes.end() || !tile) {
    new_home = game_random::cube_coord(start);
    tile = world_map::get_tile(new_home); 
  }

  std::cout << current->m_name << " found a home at " << format::vector3(new_home) << std::endl;
  // Create a worker, for now, on that tile then a city
  SpawnStep spawn_step;
  spawn_step.set_unit_type(util::enum_to_uint(UNIT_TYPE::WORKER));
  spawn_step.set_location(new_home);
  spawn_step.set_player(player_id);
  ai_shared::simulate_step(spawn_step, s_ai_buffer, BUFFER_LEN);

  // Preemptively get the id of the city that will be created in the colonize step.
  uint32_t city_id = unique_id::get_next();

  ColonizeStep colonize_step;
  colonize_step.set_location(new_home);
  colonize_step.set_player(player_id);
  ai_shared::simulate_step(colonize_step, s_ai_buffer, BUFFER_LEN);

  // TEMPORARY: Construct the barbarian and uber forge.
  ConstructionStep forge;
  forge.set_city_id(city_id);
  forge.set_production_id(util::enum_to_uint(CONSTRUCTION_TYPE::FORGE));
  forge.set_player(player_id);
  // Give it to them immediately.
  forge.set_cheat(true);
  ai_shared::simulate_step(forge, s_ai_buffer, BUFFER_LEN);
}

void EmpireConstruct::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Unable to find player to construct." << std::endl;
  }

  for (auto id : current->m_cities) {
    City* city = city::get_city(id);
    if (!city) continue;
    // Begin construction of the production type in all available cities.
    if (!city->IsConstructing()) {
      std::cout << current->m_name << " beginning construction of: " << get_construction_name(m_production_type) <<
        " in city: " << city->m_id << std::endl;
      production_queue::add(city->GetProductionQueue(), m_production_type);
    }
  }
}

void EmpireExplore::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Invalid player id. Explore decision." << std::endl;
    return;
  }

  std::cout << current->m_name << " exploring." << std::endl;
  player::for_each_player_unit(player_id, [&player_id, &current](Unit& unit) {
    sf::Vector3i coord = ai_shared::get_random_coord();
    std::cout << current->m_name << " going towards " << format::vector3(coord) << std::endl;
    MoveStep move_step;
    move_step.set_unit_id(unit.m_id);
    move_step.set_destination(coord);
    move_step.set_player(player_id);
    move_step.set_immediate(true);
    ai_shared::simulate_step(move_step, s_ai_buffer, BUFFER_LEN);
  });
}

UnitOrder reevaluate_order(uint32_t /*unit_id*/, uint32_t /*player_id*/) {
  // No reevaluation
  std::cout << "IMPLEMENT REEVALUATION!!" << std::endl;
  return UnitOrder(0, 0, AI_ORDER_TYPE::UNKNOWN);
}

void execute_order(const UnitOrder& decision, uint32_t player_id) {
  uint32_t uid = decision.m_unit_id;
  uint32_t tid = decision.m_target_id;

  std::cout << "Executing order: " << get_ai_order_name(decision.m_order) 
    << " for player_id: " << player_id << std::endl;

  // Try to execute an AI order, if it can't be execute reevaluate the order.
  // Reevaluation can occur if a unit tries to attack a unit that no longer exists.
  switch (decision.m_order) {
  case AI_ORDER_TYPE::ATTACK_UNIT:
    if (!ai_shared::attack_unit(uid, tid)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::APPROACH_UNIT:
    if (!ai_shared::approach_unit(uid, tid)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::ATTACK_CITY:
    std::cout << "Attack City not yet implemented." << std::endl;
    break;
  case AI_ORDER_TYPE::APPROACH_CITY:
    if (!ai_shared::approach_city(uid, tid)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::PILLAGE_IMPROVEMENT:
    if (!ai_shared::pillage_improvement(uid, tid)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::WANDER:
    if (!ai_shared::wander(uid)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::APPROACH_IMPROVEMENT: // ASSUMPTION: Only improve can cause this.
    if (!ai_shared::approach_improvement(uid, player_id)) /* TODO: player_id != improvement_id */ {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
  case AI_ORDER_TYPE::UNKNOWN:
  default:
    std::cout << "Unknown decision." << std::endl;
    return;
  }
}

void EmpireUnitDecisions::operator()(uint32_t player_id) {
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player id. Unit decision." << std::endl;
    return;
  }

  std::shared_ptr<AIState> state = p->m_ai_state;
  for (auto& order : state->m_orders) {
    execute_order(order, player_id);
  }

  // Clear the orders.
  state->m_orders.clear();
}

namespace empire_decisions {
  EmpireSettle s_empire_settle;
  EmpireConstruct s_empire_construct(CONSTRUCTION_TYPE::MELEE);
  EmpireExplore s_empire_explore;
  EmpireUnitDecisions s_empire_unit_decisions;
}

EmpireSettle& empire_decisions::get_settle() {
  return s_empire_settle;
}

EmpireConstruct& empire_decisions::get_construct() {
  return s_empire_construct;
}

EmpireExplore& empire_decisions::get_explore() {
  return s_empire_explore;
}

EmpireUnitDecisions& empire_decisions::get_unit_decisions() {
  return s_empire_unit_decisions;
}
