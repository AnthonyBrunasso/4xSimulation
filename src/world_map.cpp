#include "world_map.h"

#include "search.h"
#include "tile.h"

#include <unordered_map>
#include <vector>

namespace {
  static std::unordered_map<sf::Vector3i, Tile> s_map;
  static std::vector<sf::Vector3i> s_coords;
}

void world_map::build(sf::Vector3i start, uint32_t size) {
  search::range(start, size, s_coords);
  for (auto tile : s_coords) {
    s_map[tile] = Tile();
  }
}

void for_each_tile(std::function<void(const sf::Vector3i& coord, const Tile& tile)> operation) {
  for (auto tile : s_map) {
    operation(tile.first, tile.second);
  }
}

Tile* world_map::get_tile(sf::Vector3i location) {
  if (s_map.find(location) == s_map.end()) {
    return nullptr;
  }

  return &s_map[location];
}