#include "tile_costs.h"

#include <unordered_map>

#include "enum_generated.h"
#include "util.h"

namespace {
  typedef std::unordered_map<uint32_t, uint32_t> TerrainCostMap;
  TerrainCostMap s_costs;
}

void tile_costs::initialize() {
  auto check = ([](fbs::TERRAIN_TYPE terrain_type) {
    // By default the cost of all terrain types is 1.
      tile_costs::set(terrain_type, 1);
  });
  for (auto tt : fbs::EnumValuesTERRAIN_TYPE()) {
    check(tt);
  }

  tile_costs::set(fbs::TERRAIN_TYPE::MOUNTAIN, 2);
  tile_costs::set(fbs::TERRAIN_TYPE::WATER, 4);
}

uint32_t tile_costs::get(fbs::TERRAIN_TYPE id) {
  uint32_t key = any_enum(id);
  return s_costs[key];
}

void tile_costs::set(fbs::TERRAIN_TYPE id, uint32_t cost) {
  uint32_t key = any_enum(id);
  s_costs[key] = cost;
}

void tile_costs::reset() {
  s_costs.clear();
}
