#include "ai_evaluator_store.h"

namespace {
  NeedsColonize s_colonize_evaluator;
  // Barbarians only produce melee units for now.
  NeedsProduce s_produce_evaluator;
  // Barbarians only construct melee units.
  HasUnits s_has_units_evaluator;
  DiscoveredCities s_discovered_cities_evaluator;
  UnitEvaluation s_unit_evaluation;
}

NeedsColonize& evaluations::get_colonize() {
  return s_colonize_evaluator;
}

NeedsProduce& evaluations::get_produce() {
  return s_produce_evaluator;
}

HasUnits& evaluations::get_has_units() {
  return s_has_units_evaluator;
}

DiscoveredCities& evaluations::get_discovered_cities() {
  return s_discovered_cities_evaluator;
}

UnitEvaluation& evaluations::get_unit_order() {
  return s_unit_evaluation;
}

