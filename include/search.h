#pragma once

#include "Vector3.hpp"
#include "world_map.h"

#include <cstdint>
#include <vector>

namespace search {
  void range(const sf::Vector3i& start, int32_t distance, std::vector<sf::Vector3i>& coords);
  void path_to(const sf::Vector3i& start, 
      const sf::Vector3i& end, 
      world_map::TileMap& tile_map, 
      std::vector<sf::Vector3i>& coords);
}
