#include "ai_unit_decisions.h"

#include "unit.h"
#include "util.h"
#include "random.h"
#include "network_types.h"
#include "world_map.h"
#include "format.h"

#include <iostream>

namespace unit_decisions {
  UnitWander s_wander;
  UnitFight s_fight;

  char s_unit_buffer[256];

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
}

void UnitWander::operator()(uint32_t unit_id) {
  Unit* su = unit::get_unit(unit_id);
  if (!su) return;

  // Move to the improvement.
  MoveStep move_step;
  move_step.set_unit_id(unit_id);
  move_step.set_destination(unit_decisions::get_random_coord());
  move_step.set_player(su->m_owner_id);
  move_step.set_immediate(true);
  util::simulate_step(move_step, unit_decisions::s_unit_buffer);
}

void UnitFight::operator()(uint32_t unit_id) {

}

UnitWander& unit_decisions::get_wander() {
  return s_wander;
}

UnitFight& unit_decisions::get_fight() {
  return s_fight;
}