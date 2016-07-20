#include "ai_decisions.h"

#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include "step.h"
#include "tile.h"
#include "player.h"
#include "city.h"
#include "simulation.h"
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

void Settle::operator()(uint32_t player_id) {
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
  // Create a worker, for now, on that tile then a city.
  SpawnStep* spawn_step = new SpawnStep();
  spawn_step->m_unit_type = util::enum_to_uint(UNIT_TYPE::WORKER);
  spawn_step->m_location = new_home;
  spawn_step->m_player = player_id;
  simulation::process_step_from_ai(spawn_step);

  // Preemptively get the id of the city that will be created in the colonize step.
  uint32_t city_id = unique_id::get_next();

  ColonizeStep* colonize_step = new ColonizeStep();
  colonize_step->m_location = new_home;
  colonize_step->m_player = player_id;
  simulation::process_step_from_ai(colonize_step);

  // TEMPORARY: Construct the barbarian and uber forge.
  ConstructionStep* forge = new ConstructionStep();
  forge->m_city_id = city_id;
  forge->m_production_id = util::enum_to_uint(CONSTRUCTION_TYPE::FORGE);
  forge->m_player = player_id;
  // Give it to them immediately.
  forge->m_cheat = true;
  simulation::process_step_from_ai(forge);
}

void Construct::operator()(uint32_t player_id) {
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
      city->GetConstruction()->Add(m_production_type);
    }
  }
}

sf::Vector3i get_random_coord() {
    sf::Vector3i coord = game_random::cube_coord(6);
    std::cout << format::vector3(coord) << std::endl;
    Tile* tile = world_map::get_tile(coord);
    while (!tile) {
      coord = game_random::cube_coord(6);
      tile = world_map::get_tile(coord); 
    }
    return coord;
}

void Explore::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Invalid player id. Explore decision." << std::endl;
    return;
  }

  std::cout << current->m_name << " exploring." << std::endl;
  player::for_each_player_unit(player_id, [&player_id, &current](Unit& unit) {
    sf::Vector3i coord = get_random_coord();
    std::cout << current->m_name << " going towards " << format::vector3(coord) << std::endl;
    MoveStep* move_step = new MoveStep();
    move_step->m_unit_id = unit.m_unique_id;
    move_step->m_destination = coord;
    move_step->m_player = player_id; 
    simulation::process_step_from_ai(move_step);
  });
}

bool attack_unit(uint32_t unit_id, uint32_t target_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  Unit* tu = units::get_unit(target_id);
  if (!su || !tu) {
    if (!tu) std::cout << "Target is gone id: " << target_id << std::endl;
    return false; 
  }

  // Don't attempt an attack if out of action points.
  if (!su->m_action_points) {
    std::cout << "No action points to attack" << std::endl;
    // Return true, decision is a success, just out of action points.
    return true;
  }
  
  AttackStep* attack_step = new AttackStep();
  attack_step->m_attacker_id = unit_id;
  attack_step->m_defender_id = target_id;
  attack_step->m_player = player_id;
  simulation::process_step_from_ai(attack_step);
  return true;
}

void approach(uint32_t unit_id, 
    uint32_t player_id, 
    const sf::Vector3i& start, 
    const sf::Vector3i& location) {
  auto find_tiles = [&location](const Tile& tile) -> bool {
    if (tile.m_city_id) return false;
    if (!tile.m_unit_ids.empty()) return false;
    return true;
  };

  std::vector<sf::Vector3i> path = search::path_to(start, location, world_map::get_map(), find_tiles);

  // Erase the first node, the node being stood upon, and the last node, the target city or unit.
  if (path.size() > 2) {
    path.erase(path.begin());
    path.pop_back();
  }
  // Otherwise return, the unit is close enough already.
  else {
    std::cout << "Unit is close enough to it's target." << std::endl;
    return;
  }

  SetPathStep* move_step = new SetPathStep();
  move_step->m_unit_id = unit_id;
  move_step->m_path = path;
  move_step->m_player = player_id;
  simulation::process_step_from_ai(move_step);
}


bool approach_unit(uint32_t unit_id, uint32_t target_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  Unit* tu = units::get_unit(target_id);
  if (!su || !tu) {
    if (!tu) std::cout << "Target unit is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, player_id, su->m_location, tu->m_location);
  // Try to attack the unit.
  attack_unit(unit_id, target_id, player_id);
  return true;
}

bool approach_city(uint32_t unit_id, uint32_t target_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  City* tc = city::get_city(target_id);
  if (!su || !tc) {
    if (!tc) std::cout << "Target city is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, player_id, su->m_location, tc->m_location);
  return true;
}

bool pillage_improvement(uint32_t unit_id, uint32_t target_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  Improvement* ti = improvement::get_improvement(target_id);
  if (!su || !ti) {
    if (!ti) std::cout << "Target improvement is gone id: " << target_id << std::endl;
    return false; 
  }

  // Don't attempt an attack if out of action points.
  if (!su->m_action_points) {
    // Return true, decision is a success, just out of action points.
    return true;
  }
 
  PillageStep* pillage_step = new PillageStep();
  pillage_step->m_player = player_id;
  pillage_step->m_unit = unit_id;
  simulation::process_step_from_ai(pillage_step);
  return true;
}

bool wander(uint32_t unit_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  if (!su) {
    return false;
  }

  // Move to the improvement.
  MoveStep* move_step = new MoveStep();
  move_step->m_unit_id = unit_id;
  move_step->m_destination = get_random_coord();
  move_step->m_player = player_id;
  simulation::process_step_from_ai(move_step);

  return true;
}

bool approach_improvement(uint32_t unit_id, uint32_t target_id, uint32_t player_id) {
  Unit* su = units::get_unit(unit_id);
  Improvement* ti = improvement::get_improvement(target_id);
  if (!su || !ti) {
    if (!ti) std::cout << "Target improvement is gone id: " << target_id << std::endl;
    return false; 
  } 

  // Move to the improvement.
  MoveStep* move_step = new MoveStep();
  move_step->m_unit_id = unit_id;
  move_step->m_destination = ti->m_location;
  move_step->m_player = player_id;
  simulation::process_step_from_ai(move_step);

  // Try to pillage it.
  pillage_improvement(unit_id, target_id, player_id);
  return true;
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
    if (!attack_unit(uid, tid, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::APPROACH_UNIT:
    if (!approach_unit(uid, tid, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::ATTACK_CITY:
    std::cout << "Attack City not yet implemented." << std::endl;
    break;
  case AI_ORDER_TYPE::APPROACH_CITY:
    if (!approach_city(uid, tid, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::PILLAGE_IMPROVEMENT:
    if (!pillage_improvement(uid, tid, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::WANDER:
    if (!wander(uid, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
    break;
  case AI_ORDER_TYPE::APPROACH_IMPROVEMENT: // ASSUMPTION: Only improve can cause this.
    if (!approach_improvement(uid, player_id, player_id)) {
      execute_order(reevaluate_order(uid, player_id), player_id);
    }
  case AI_ORDER_TYPE::UNKNOWN:
  default:
    std::cout << "Unknown decision." << std::endl;
    return;
  }
}

void UnitDecision::operator()(uint32_t player_id) {
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player id. Unit decision." << std::endl;
    return;
  }

  AIState* state = p->m_ai_state;
  for (auto& order : state->m_orders) {
    execute_order(order, player_id);
  }

  // Clear the orders.
  state->m_orders.clear();
}
