#include "ai_barbarians.h"

#include "ai_evaluations.h"
#include "ai_decisions.h"
#include "ai_state.h"
#include "player.h"
#include "dtree.h"
#include "game_types.h"

#include <iostream>
#include <vector>

namespace {
  DTree* s_macro_dtree = nullptr;
  DTree* s_micro_dtree = nullptr;
  // TODO: Move all these into its own modules
  NeedsColonize s_colonize_evaluator;
  // Barbarians only produce melee units for now.
  NeedsProduce s_produce_evaluator;
  Settle s_settle_decision;
  Explore s_explore_decision;
  // Barbarians only construct melee units.
  Construct s_construct_decision(CONSTRUCTION_TYPE::MELEE);
  HasUnits s_has_units_evaluator;
  DiscoveredCities s_discovered_cities_evaluator;
  UnitDecision s_unit_decision;
  UnitEvaluation s_unit_evaluation;
  // TODO end here
  std::vector<uint32_t> s_player_ids;
}

// Create the barbarian behavior.
void barbarians::initialize() { 
  // Create macro tree. Check if the player needs to colonize.
  DNode* node = new DNode(nullptr, &s_colonize_evaluator);

  // If they do create a city.
  node->m_right = new DNode(&s_settle_decision, nullptr);
  // Otherwise evaluate the production needs of the city.
  node->m_left = new DNode(nullptr, &s_produce_evaluator);
  DNode*& produce_eval = node->m_left;
  // Construct something in the city.
  produce_eval->m_right = new DNode(&s_construct_decision, nullptr);

  // Create micro tree.
  DNode* mnode = new DNode(nullptr, &s_has_units_evaluator);
  
  mnode->m_right = new DNode(nullptr, &s_discovered_cities_evaluator);
  DNode*& discovered_eval = mnode->m_right;
  // If there are no discovered cities send all units to explore.
  discovered_eval->m_left = new DNode(&s_explore_decision, nullptr);
  // Otherwise evaluate each units order.
  discovered_eval->m_right = new DNode(nullptr, &s_unit_evaluation);
  DNode*& attack_eval = discovered_eval->m_right;
  // Execute all unit orders.
  attack_eval->m_right = new DNode(&s_unit_decision, nullptr);
  // If unit orders can't be evaluated, for whatever reason, just explore.
  attack_eval->m_left = new DNode(&s_explore_decision, nullptr);

  // Construct the barbarians decision tree.
  s_macro_dtree = new DTree(node);
  s_micro_dtree = new DTree(mnode);
}

void barbarians::destroy() {
  delete s_macro_dtree;
  delete s_micro_dtree;
}

// Lets the barbarians execute their turn. 
void barbarians::pillage_and_plunder() {
  for (auto id : s_player_ids) {
    s_macro_dtree->make_decision(id);
    s_micro_dtree->make_decision(id);
  }
}

// Set the barbarians player id.
void barbarians::set_player_id(uint32_t player_id) {
  std::cout << "Barbarians have joined the battle" << std::endl;
  s_player_ids.push_back(player_id);
  // Get the player and build its state object
  Player* p = player::get_player(player_id);
  if (!p) return;
  p->m_ai_state = new AIState();
}

