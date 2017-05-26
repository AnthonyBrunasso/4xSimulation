#include "ai_empire_decisions.h"
#include "ai_shared.h"

#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include "tile.h"
#include "player.h"
#include "city.h"
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

#include "step_generated.h"

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
  flatbuffers::Offset<fbs::SpawnStep> spawn_step;
  uint32_t unit_type = (util::enum_to_uint(UNIT_TYPE::WORKER));
  fbs::v3i location(new_home.x, new_home.y, new_home.z);
  spawn_step = fbs::CreateSpawnStep(ai_shared::GetFBB(), unit_type, &location, player_id);
  ai_shared::simulate_step(fbs::StepUnion::SpawnStep, spawn_step.Union());

  // Preemptively get the id of the city that will be created in the colonize step.
  uint32_t city_id = unique_id::get_next();

  flatbuffers::Offset<fbs::ColonizeStep> colonize_step;
  fbs::v3i loc(new_home.x, new_home.y, new_home.z);
  colonize_step = fbs::CreateColonizeStep(ai_shared::GetFBB(), &loc, player_id);
  ai_shared::simulate_step(fbs::StepUnion::ColonizeStep, colonize_step.Union());

  // TEMPORARY: Construct the barbarian and uber forge.
  flatbuffers::Offset<fbs::ConstructionStep> forge;
  uint32_t production_id = (util::enum_to_uint(CONSTRUCTION_TYPE::FORGE));
  bool cheat = true;
  forge = fbs::CreateConstructionStep(ai_shared::GetFBB(), city_id, production_id, cheat, player_id);
  ai_shared::simulate_step(fbs::StepUnion::ConstructionStep, forge.Union());
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
    flatbuffers::Offset<fbs::MoveStep> move_step;
    uint32_t unit_id = (unit.m_id);
    fbs::v3i dest(coord.x, coord.y, coord.z);
    uint32_t player_id = (player_id);
    bool immediate = true;
    bool avoid_city = (true);
    bool avoid_unit = (true);
    bool require_ownership = true;
    move_step = fbs::CreateMoveStep(ai_shared::GetFBB(), unit_id, &dest, player_id, immediate, avoid_unit, avoid_city, require_ownership);
    ai_shared::simulate_step(fbs::StepUnion::MoveStep, move_step.Union());
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

void EmpireEndTurn::operator()(uint32_t player_id) {
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player id. Unit decision." << std::endl;
    return;
  }

  std::shared_ptr<AIState>& state = p->m_ai_state;
  state->m_micro_done = true;
}

void EmpireFortifyUnit::operator()(uint32_t player_id) {
  // no-op for now.
  std::cout << "Would have fortified this unit: " << m_state->m_idle_units.back() << std::endl;
}

void EmpireApproach::operator()(uint32_t player_id) {
  Unit* u = unit::get_unit(m_state->m_idle_units.back());
  if (!u) return;

  ai_shared::approach(m_state->m_idle_units.back(), m_state->m_target_location);
  std::cout << "Unit: " << m_state->m_idle_units.back() << " approaching location: " << format::vector3(m_state->m_target_location) 
    << "from: " << format::vector3(u->m_location) << std::endl;
}

void EmpireAttack::operator()(uint32_t player_id) {
  ai_shared::attack_unit(m_state->m_idle_units.back(), m_state->m_threats.back());
}

 void EmpireSiege::operator()(uint32_t player_id) {
  ai_shared::attack_city(m_state->m_idle_units.back(), m_state->m_target_city);
}

void EmpirePillage::operator()(uint32_t player_id){
  ai_shared::pillage_improvement(m_state->m_idle_units.back(), m_state->m_pillage_targets.back());
  std::cout << "Unit: " << m_state->m_idle_units.back() << " pillaging improvement: " << m_state->m_pillage_targets.back() << std::endl;
}

void EmpireWander::operator()(uint32_t player_id) {
  ai_shared::wander(m_state->m_idle_units.back());
  std::cout << "Unit: " << m_state->m_idle_units.back() << " wandering..." << std::endl;
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

EmpireEndTurn& empire_decisions::decide_endturn() {
  static EmpireEndTurn s_end_turn;
  return s_end_turn;
}

EmpireFortifyUnit& empire_decisions::decide_fortify() {
  static EmpireFortifyUnit s_fortify;
  return s_fortify;
}

EmpireApproach& empire_decisions::decide_approach() {
  static EmpireApproach s_approach;
  return s_approach;
}

EmpireAttack& empire_decisions::decide_attack() {
  static EmpireAttack s_attack;
  return s_attack;
}

EmpireSiege& empire_decisions::decide_siege() {
  static EmpireSiege s_siege;
  return s_siege;
}

EmpirePillage& empire_decisions::decide_pillage() {
  static EmpirePillage s_pillage;
  return s_pillage;
}

EmpireWander& empire_decisions::decide_wander() {
  static EmpireWander s_wander;
  return s_wander;
}
