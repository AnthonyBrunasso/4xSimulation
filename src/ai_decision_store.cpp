#include "ai_decision_store.h"

#include "game_types.h"

namespace {
  Settle s_settle_decision;
  Explore s_explore_decision;
  Construct s_construct_decision(CONSTRUCTION_TYPE::MELEE);
  UnitDecision s_unit_decision;
}

Settle& decisions::get_settle() {
  return s_settle_decision;
}

Explore& decisions::get_explore() {
  return s_explore_decision;
}

Construct& decisions::get_construct() {
  return s_construct_decision;
}

UnitDecision& decisions::get_unit() {
  return s_unit_decision;
}

