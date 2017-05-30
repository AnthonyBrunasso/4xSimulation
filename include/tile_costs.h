# pragma once



#include <cstdint>

namespace fbs {
  enum class TERRAIN_TYPE : uint32_t;
}

namespace tile_costs {
  // Sets up various tile costs for terrain types.
  void initialize();

  uint32_t get(fbs::TERRAIN_TYPE id);
  void set(fbs::TERRAIN_TYPE id, uint32_t cost);

  void reset();
}
