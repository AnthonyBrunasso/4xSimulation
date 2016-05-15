#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <functional>

class Tile;

namespace world_map {
  void build(sf::Vector3i start, uint32_t size);
  void for_each_tile(std::function<void(const Tile& tile)> operation);

  Tile* get_tile(sf::Vector3i location);
}