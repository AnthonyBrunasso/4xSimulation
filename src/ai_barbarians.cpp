#include "ai_barbarians.h"

#include "ai_evaluations.h"
#include "ai_decisions.h"
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
  Construct s_construct_decision(CONSTRUCTION_TYPE::MELEE);
  HasUnits s_has_units_evaluator;
  DiscoveredCities s_discovered_cities_evaluator;
  // TODO end here
  uint32_t s_player_id;
}

// Create the barbarian behavior.
void barbarians::initialize() { 
  // Create macro tree.
  DNode* node = new DNode(nullptr, &s_colonize_evaluator);

  node->m_right = new DNode(&s_settle_decision, nullptr);
  node->m_left = new DNode(nullptr, &s_produce_evaluator);
  DNode*& produce_eval = node->m_left;
  produce_eval->m_right = new DNode(&s_construct_decision, nullptr);

  // Create micro tree.
  DNode* mnode = new DNode(nullptr, &s_has_units_evaluator);
  
  mnode->m_right = new DNode(nullptr, &s_discovered_cities_evaluator);
  DNode*& discovered_eval = mnode->m_right;
  discovered_eval->m_left = new DNode(&s_explore_decision, nullptr);

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
  s_macro_dtree->make_decision(s_player_id);
  s_micro_dtree->make_decision(s_player_id);
}

// Set the barbarians player id.
void barbarians::set_player_id(uint32_t player_id) {
  std::cout << "Barbarians have joined the battle" << std::endl;
  s_player_id = player_id;
}

