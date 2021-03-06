#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include "Vector3.hpp"
#include "tile.h"

namespace world_map {
  typedef std::unordered_map<sf::Vector3i, Tile> TileMap;

  void build(sf::Vector3i start, uint32_t size);
  bool load_file_fb(const std::string& name);
  bool load_file(const std::string& name);
  bool save_file_fb(const char*);
  bool save_file(const char*);
  void for_each_tile(std::function<void(const sf::Vector3i& coord, const Tile& tile)> operation);

  bool remove_unit(const sf::Vector3i& location, uint32_t unit_id);
  bool add_unit(const sf::Vector3i& location, uint32_t unit_id);
  uint32_t move_unit(uint32_t unit_id, uint32_t distance);

  TileMap& get_map();
  uint32_t get_map_size();

  Tile* get_tile(sf::Vector3i location);
  uint32_t tile_owner(const Tile& tile);

  void reset();
}
