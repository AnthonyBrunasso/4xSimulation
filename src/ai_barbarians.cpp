#include "ai_barbarians.h"

#include "ai_empire_trees.h"
#include "player.h"
#include "dtree.h"
#include "game_types.h"

#include <iostream>
#include <vector>

namespace {
  std::vector<uint32_t> s_player_ids;
}

// Create the barbarian behavior.
void barbarians::initialize() { 

}

void barbarians::destroy() {
 
}

// Lets the barbarians execute their turn. 
void barbarians::pillage_and_plunder(uint32_t player_id) {
  for (auto id : s_player_ids) {
    if (id == player_id) {
      empire_trees::get_primitive_macro().make_decision(id);
      empire_trees::get_primitive_micro().make_decision(id);
    }
  }
}

// Set the barbarians player id.
void barbarians::set_player_id(uint32_t player_id) {
  std::cout << "Barbarians have joined the battle" << std::endl;
  s_player_ids.push_back(player_id);
  // Get the player and build its state object
  Player* p = player::get_player(player_id);
  if (!p) return;
  p->m_ai_state = std::make_shared<AIState>();
}

