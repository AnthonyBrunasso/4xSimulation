#include "tile_costs.h"

#include "util.h"

#include <unordered_map>

namespace {
  typedef std::unordered_map<uint32_t, uint32_t> TerrainCostMap;
  TerrainCostMap s_costs;
}

void tile_costs::initialize() {
  for_each_terrain_type([](TERRAIN_TYPE terrain_type) {
    // By default the cost of all terrain types is 1.
      tile_costs::set(terrain_type, 1);
  });

  tile_costs::set(TERRAIN_TYPE::MOUNTAIN, 2);
  tile_costs::set(TERRAIN_TYPE::WATER, 4);
}

uint32_t tile_costs::get(TERRAIN_TYPE id) {
  uint32_t key = util::enum_to_uint(id);
  return s_costs[key];
}

void tile_costs::set(TERRAIN_TYPE id, uint32_t cost) {
  uint32_t key = util::enum_to_uint(id);
  s_costs[key] = cost;
}

