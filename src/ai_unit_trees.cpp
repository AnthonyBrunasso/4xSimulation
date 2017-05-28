#include "ai_unit_trees.h"

#include <memory>

#include "ai_unit_decisions.h"
#include "ai_unit_evaluations.h"
#include "dtree.h"

namespace unit_trees {
  std::unique_ptr<DTree> s_destructive(nullptr); 
}

DTree& unit_trees::get_destructive() {
  if (s_destructive) return *s_destructive;

  DNode* node = new DNode(nullptr, &unit_evaluations::get_find_near());

  node->m_left = new DNode(&unit_decisions::get_wander(), nullptr);
  node->m_right = new DNode(&unit_decisions::get_fight(), nullptr);
  s_destructive.reset(new DTree(node));

  return *s_destructive;
}
