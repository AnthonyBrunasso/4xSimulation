#include "ai_monster.h"

#include <iostream>

#include "player.h"
#include "ai_unit_trees.h"
#include "dtree.h"
#include "unit.h"

void monster::execute_turn(uint32_t player_id) {
  std::cout << "Executing monster turn." << std::endl;
  
  player::for_each_player_unit(player_id, [](Unit& unit) {
    unit_trees::get_destructive().make_decision(unit.m_id);
  });
}
