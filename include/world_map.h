#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <functional>

class Tile;

namespace world_map {
  void build(sf::Vector3i start, uint32_t size);
  void for_each_tile(std::function<void(const sf::Vector3i& coord, const Tile& tile)> operation);

  bool remove_unit(const sf::Vector3i& location, uint32_t unit_id);
  bool add_unit(const sf::Vector3i& location, uint32_t unit_id);
  uint32_t move_unit(uint32_t unit_id, uint32_t distance);

  Tile* get_tile(sf::Vector3i location);
}