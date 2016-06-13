#include "ai_barbarians.h"

#include "ai_evaluations.h"
#include "ai_decisions.h"
#include "dtree.h"
#include "game_types.h"

#include <iostream>

namespace {
  DTree* s_dtree = nullptr;
  NeedsColonize s_colonize_evaluator;
  // Barbarians only produce melee units for now.
  NeedsProduce s_produce_evaluator;
  Settle s_settle_decision;
  Construct s_construct_decision(CONSTRUCTION_TYPE::MELEE);
  uint32_t s_player_id;
}

// Create the barbarian behavior.
void barbarians::initialize() { 
  DNode* node = new DNode(nullptr, &s_colonize_evaluator);

  node->m_right = new DNode(&s_settle_decision, nullptr);
  node->m_left = new DNode(nullptr, &s_produce_evaluator);
  DNode*& produce_eval = node->m_left;

  produce_eval->m_right = new DNode(&s_construct_decision, nullptr);

  // Construct the barbarians decision tree.
  s_dtree = new DTree(node);
}

void barbarians::destroy() {
  delete s_dtree;
}

// Lets the barbarians execute their turn. 
void barbarians::pillage_and_plunder() {
  s_dtree->make_decision(s_player_id);
}

// Set the barbarians player id.
void barbarians::set_player_id(uint32_t player_id) {
  std::cout << "Barbarians have joined the battle" << std::endl;
  s_player_id = player_id;
}

