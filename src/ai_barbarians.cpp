#include "ai_barbarians.h"

#include "ai_evaluations.h"
#include "ai_evaluator_store.h"
#include "ai_decisions.h"
#include "ai_decision_store.h"
#include "ai_state.h"
#include "player.h"
#include "dtree.h"
#include "game_types.h"

#include <iostream>
#include <vector>

namespace {
  DTree* s_macro_dtree = nullptr;
  DTree* s_micro_dtree = nullptr;
  std::vector<uint32_t> s_player_ids;
}

// Create the barbarian behavior.
void barbarians::initialize() { 
  // Create macro tree. Check if the player needs to colonize.
  DNode* node = new DNode(nullptr, &evaluations::get_colonize());

  // If they do create a city.
  node->m_right = new DNode(&decisions::get_settle(), nullptr);
  // Otherwise evaluate the production needs of the city.
  node->m_left = new DNode(nullptr, &evaluations::get_produce());
  DNode*& produce_eval = node->m_left;
  // Construct something in the city.
  produce_eval->m_right = new DNode(&decisions::get_construct(), nullptr);

  // Create micro tree.
  DNode* mnode = new DNode(nullptr, &evaluations::get_has_units());
 
  // TODO: Check proximity of units before checking if any cities have been discovered. 
  
  mnode->m_right = new DNode(nullptr, &evaluations::get_discovered_cities());
  DNode*& discovered_eval = mnode->m_right;
  // If there are no discovered cities send all units to explore.
  discovered_eval->m_left = new DNode(&decisions::get_explore(), nullptr);
  // Otherwise evaluate each units order.
  discovered_eval->m_right = new DNode(nullptr, &evaluations::get_unit_order());
  DNode*& attack_eval = discovered_eval->m_right;
  // Execute all unit orders.
  attack_eval->m_right = new DNode(&decisions::get_unit(), nullptr);
  // If unit orders can't be evaluated, for whatever reason, just explore.
  attack_eval->m_left = new DNode(&decisions::get_explore(), nullptr);

  // Construct the barbarians decision tree.
  s_macro_dtree = new DTree(node);
  s_micro_dtree = new DTree(mnode);
}

void barbarians::destroy() {
  delete s_macro_dtree;
  delete s_micro_dtree;
}

// Lets the barbarians execute their turn. 
void barbarians::pillage_and_plunder(uint32_t player_id) {
  for (auto id : s_player_ids) {
    if (id == player_id) {
      s_macro_dtree->make_decision(id);
      s_micro_dtree->make_decision(id);
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

