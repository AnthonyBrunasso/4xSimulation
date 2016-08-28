#include "ai_empire_trees.h"

#include <memory>

#include "ai_empire_evaluations.h"
#include "ai_empire_decisions.h"

namespace empire_trees {
  std::unique_ptr<DTree> s_primitive_macro(nullptr);
  std::unique_ptr<DTree> s_primitive_micro(nullptr);
}

DTree& empire_trees::get_primitive_macro() {
  if (s_primitive_macro) return *s_primitive_macro;

  // Create macro tree. Check if the player needs to colonize.
  DNode* node = new DNode(nullptr, &empire_evaluations::get_colonize());

  // If they do create a city.
  node->m_right = new DNode(&empire_decisions::get_settle(), nullptr);
  // Otherwise evaluate the production needs of the city.
  node->m_left = new DNode(nullptr, &empire_evaluations::get_produce());
  DNode*& produce_eval = node->m_left;
  // Construct something in the city.
  produce_eval->m_right = new DNode(&empire_decisions::get_construct(), nullptr);
  s_primitive_macro.reset(new DTree(node));

  return *s_primitive_macro;
}

DTree& empire_trees::get_primitive_micro() {
  if (s_primitive_micro) return *s_primitive_micro;


  // Create micro tree.
  DNode* mnode = new DNode(nullptr, &empire_evaluations::get_units());

  // TODO: Check proximity of units before checking if any cities have been discovered. 

  mnode->m_right = new DNode(nullptr, &empire_evaluations::get_cities());
  DNode*& discovered_eval = mnode->m_right;
  // If there are no discovered cities send all units to explore.
  discovered_eval->m_left = new DNode(&empire_decisions::get_explore(), nullptr);
  // Otherwise evaluate each units order.
  discovered_eval->m_right = new DNode(nullptr, &empire_evaluations::get_unit_order());
  DNode*& attack_eval = discovered_eval->m_right;
  // Execute all unit orders.
  attack_eval->m_right = new DNode(&empire_decisions::get_unit_decisions(), nullptr);
  // If unit orders can't be evaluated, for whatever reason, just explore.
  attack_eval->m_left = new DNode(&empire_decisions::get_explore(), nullptr);
  s_primitive_micro.reset(new DTree(mnode));

  return *s_primitive_micro;
}