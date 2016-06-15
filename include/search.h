#pragma once

#include "Vector3.hpp"
#include "world_map.h"

#include <cstdint>
#include <vector>
#include <functional>

namespace search {
  void range(const sf::Vector3i& start, int32_t distance, std::vector<sf::Vector3i>& coords);
  void path_to(const sf::Vector3i& start, 
      const sf::Vector3i& end, 
      world_map::TileMap& tile_map, 
      std::vector<sf::Vector3i>& coords);

  // Returns true if any tile within the depth of a breadth first search meets the
  // criteria given by the comparator.
  bool bfs(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const Tile& tile)> comparator);
}
