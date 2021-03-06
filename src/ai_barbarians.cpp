#include "ai_barbarians.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "ai_empire_trees.h"
#include "ai_state.h"
#include "dtree.h"
#include "player.h"

namespace {
  std::vector<uint32_t> s_player_ids;
  bool s_macro_behavior;
}

// Create the barbarian behavior.
void barbarians::initialize() { 

}

void barbarians::reset() {
  s_player_ids.clear();
  s_macro_behavior = true;
}

// Lets the barbarians execute their turn. 
void barbarians::pillage_and_plunder(uint32_t player_id) {
  Player* p = player::get_player(player_id);
  if (!p) return;

  if (!p->m_ai_state) return;

  std::cout << "Process turn for barbarian " << player_id << std::endl;

  if (s_macro_behavior) {
    std::cout << "Barbarian macro is active" << std::endl;
    empire_trees::get_primitive_macro().make_decision(player_id);
  }
  
  std::cout << "Barbarian micro is active" << std::endl;
  p->m_ai_state->m_micro_done = false;
  while (!p->m_ai_state->m_micro_done) {
    TurnState t;
    empire_trees::get_primitive_micro(&t).make_decision(player_id);
    // Stop once a noop has occurred
    if (t.m_noop) break;
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

void barbarians::disable_macro_behavior() {
  s_macro_behavior = false;
}

