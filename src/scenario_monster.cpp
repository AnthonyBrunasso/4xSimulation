#include "scenario_monster.h"

#include <iostream>

#include "status_effect.h"
#include "player.h"
#include "unit.h"

uint32_t INVALID_MONSTER_ID = 0xffffffff;

namespace scenario_monster {
  uint32_t s_monster_id = INVALID_MONSTER_ID;
  uint32_t s_channeled_power = 0;
}

void scenario_monster::start() {
  // Add a monster player.
  if (s_monster_id == INVALID_MONSTER_ID) {
    s_monster_id = player::create_ai(AI_TYPE::MONSTER); 
  }
  sf::Vector3i loc(0, 0, 0);
  auto summon_monster = [loc]() {
    unit::create(UNIT_TYPE::MONSTER, loc, s_monster_id);
  };
  status_effect::inject_end(summon_monster);
  status_effect::create(STATUS_TYPE::SUMMONING_MONSTER, loc);
}

void scenario_monster::process() {
  // For each barbarian tribe channel energy
  uint32_t count = 0;
  auto player_count = [&count](Player& player) {
    if (player.m_ai_type == AI_TYPE::BARBARIAN) ++count;
  };
  player::for_each_player(player_count);
  s_channeled_power += count;
}

uint32_t scenario_monster::get_monster_id() {
  return s_monster_id;
}

void scenario_monster::debug_print() {
  std::cout << "Monster has generated " << s_channeled_power << " power." << std::endl;
}
