#pragma once

#include "game_types.h"

#include <functional>

class CombatStats;

namespace fbs {
  enum class UNIT_TYPE : uint32_t;
}

// Unit definitions will contain some initial stats for certain types of unit.
namespace unit_definitions {
  // This function will contain prebuilt unit definitions.
  void initialize();
  void reset();
  CombatStats* get(fbs::UNIT_TYPE id);
  void add(fbs::UNIT_TYPE id, const CombatStats& stats);

  void for_each_definition(
      std::function<void(fbs::UNIT_TYPE type, const CombatStats& stats)> operation);
}
