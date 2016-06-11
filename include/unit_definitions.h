#pragma once

#include "game_types.h"
#include "combat.h"

#include <functional>

// Unit definitions will contain some initial stats for certain types of unit.
namespace unit_definitions {
  // This function will contain prebuilt unit definitions.
  void initialize();
  CombatStats* get(UNIT_TYPE id);
  void add(UNIT_TYPE id, const CombatStats& stats);

  void for_each_definition(
      std::function<void(UNIT_TYPE type, const CombatStats& stats)> operation);
}
