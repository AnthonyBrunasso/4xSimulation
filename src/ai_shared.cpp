#include "ai_shared.h"

#include <iostream>

#include "unit.h"
#include "city.h"
#include "improvement.h"
#include "random.h"
#include "world_map.h"
#include "format.h"

namespace ai_shared {
  const size_t BUFFER_LEN = 256;
  char s_ai_buffer[BUFFER_LEN];
}

bool ai_shared::attack_unit(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Unit* tu = unit::get_unit(target_id);
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
  
  AttackStep attack_step;
  attack_step.set_attacker_id(unit_id);
  attack_step.set_defender_id(target_id);
  attack_step.set_player(su->m_owner_id);
  simulate_step(attack_step, s_ai_buffer, BUFFER_LEN);
  return true;

}

bool ai_shared::attack_city(uint32_t unit_id, uint32_t city_id)
{
  Unit* u = unit::get_unit(unit_id);
  if (!u) return false;

  SiegeStep siege_step;
  siege_step.set_city(city_id);
  siege_step.set_player(u->m_owner_id);
  siege_step.set_unit(u->m_id);
  simulate_step(siege_step, s_ai_buffer, BUFFER_LEN);
  return true;
}

void ai_shared::approach(uint32_t unit_id, const sf::Vector3i& location) {
  Unit* u = unit::get_unit(unit_id);
  if (!u) return;
  MoveStep move_step;
  move_step.set_unit_id(unit_id);
  move_step.set_player(u->m_owner_id);
  move_step.set_destination(location);
  move_step.set_immediate(true);
  move_step.set_avoid_city(true);
  move_step.set_avoid_unit(true);
  simulate_step(move_step, s_ai_buffer, BUFFER_LEN);
}

bool ai_shared::approach_unit(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Unit* tu = unit::get_unit(target_id);
  if (!su || !tu) {
    if (!tu) std::cout << "Target unit is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, tu->m_location);
  // Try to attack the unit.
  attack_unit(unit_id, target_id);
  return true;
}

bool ai_shared::approach_city(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  City* tc = city::get_city(target_id);
  if (!su || !tc) {
    if (!tc) std::cout << "Target city is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, tc->m_location);
  return true;
}

bool ai_shared::pillage_improvement(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
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
 
  PillageStep pillage_step;
  
  pillage_step.set_player(su->m_owner_id);
  pillage_step.set_unit(unit_id);
  simulate_step(pillage_step, s_ai_buffer, BUFFER_LEN);
  return true;
}

bool ai_shared::wander(uint32_t unit_id) {
  Unit* su = unit::get_unit(unit_id);
  if (!su) {
    return false;
  }

  // Move to the improvement.
  MoveStep move_step;
  move_step.set_unit_id(unit_id);
  move_step.set_destination(get_random_coord());
  move_step.set_player(su->m_owner_id);
  move_step.set_immediate(true);
  move_step.set_avoid_city(true);
  move_step.set_avoid_unit(true);
  simulate_step(move_step, s_ai_buffer, BUFFER_LEN);

  return true;
}

bool ai_shared::approach_improvement(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Improvement* ti = improvement::get_improvement(target_id);
  if (!su || !ti) {
    if (!ti) std::cout << "Target improvement is gone id: " << target_id << std::endl;
    return false; 
  } 

  // Move to the improvement.
  MoveStep move_step;
  move_step.set_unit_id(unit_id);
  move_step.set_destination(ti->m_location);
  move_step.set_player(su->m_owner_id);
  move_step.set_immediate(true);
  move_step.set_avoid_city(true);
  move_step.set_avoid_city(true);
  simulate_step(move_step, s_ai_buffer, BUFFER_LEN);
  // Try to pillage it.
  pillage_improvement(unit_id, target_id);
  return true;
}

sf::Vector3i ai_shared::get_random_coord() {
  sf::Vector3i coord = game_random::cube_coord(6);
  std::cout << format::vector3(coord) << std::endl;
  Tile* tile = world_map::get_tile(coord);
  while (!tile) {
    coord = game_random::cube_coord(6);
    tile = world_map::get_tile(coord); 
  }
  return coord;
}



