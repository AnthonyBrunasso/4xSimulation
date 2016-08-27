#include "scenario_monster.h"

#include <iostream>
#include "status_effect.h"
#include "player.h"

namespace scenario_monster {
  uint32_t s_channeled_power = 0;
}

void scenario_monster::start() {
  status_effect::create(STATUS_TYPE::SUMMONING_MONSTER, sf::Vector3i(0, 0, 0));
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

void scenario_monster::debug_print() {
  std::cout << "Monster has generated " << s_channeled_power << " power." << std::endl;
}
