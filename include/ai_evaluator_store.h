#pragma once

#include "ai_evaluations.h"

namespace evaluations {
  NeedsColonize& get_colonize();
  NeedsProduce& get_produce();
  HasUnits& get_has_units();
  DiscoveredCities& get_discovered_cities();
  UnitEvaluation& get_unit_order();
}
