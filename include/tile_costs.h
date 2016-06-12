# pragma once

#include "game_types.h"

#include <cstdint>

namespace tile_costs {
  // Sets up various tile costs for terrain types.
  void initialize();

  uint32_t get(TERRAIN_TYPE id);
  void set(TERRAIN_TYPE id, uint32_t cost);
}
