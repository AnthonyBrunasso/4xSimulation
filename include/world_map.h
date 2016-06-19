#pragma once

#include "Vector3.hpp"

#include <unordered_map>
#include <cstdint>
#include <functional>
#include <string>

class Tile;

namespace world_map {
  typedef std::unordered_map<sf::Vector3i, Tile> TileMap;

  void build(sf::Vector3i start, uint32_t size);
  bool load_file(const std::string& name);
  void for_each_tile(std::function<void(const sf::Vector3i& coord, const Tile& tile)> operation);

  bool remove_unit(const sf::Vector3i& location, uint32_t unit_id);
  bool add_unit(const sf::Vector3i& location, uint32_t unit_id);
  uint32_t move_unit(uint32_t unit_id, uint32_t distance);

  TileMap& get_map();
  uint32_t get_map_size();

  Tile* get_tile(sf::Vector3i location);
}
