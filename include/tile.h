#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Vector3.hpp"

#include "resources.h"

namespace fbs {
  enum class TERRAIN_TYPE : uint32_t;
  enum class RESOURCE_TYPE : uint32_t;
}

// Tiles contain -
// * terrain id: to identify any special geogrpahic properties of the tile
// * array of entity ids: to identify buildings, unit, etc that are contained on the tile
class Tile {
public:
  Tile();
  Tile(sf::Vector3i location);

  fbs::TERRAIN_TYPE m_terrain_type;

  // Multiple unit can be contained on a tile, ex: Worker and warrior
  std::vector<uint32_t> m_unit_ids;
  // Only one city can be on a tile at once
  uint32_t m_city_id;
  // The cost to navigate over this tile
  uint32_t m_path_cost;
  // magical "hut" that yields bonus rewards when walked upon
  bool m_discover_bonus;
  // Tiles can contain resources
  Resource m_resource;
  // Improvements contained on this tile
  std::vector<uint32_t> m_improvement_ids;
  sf::Vector3i m_location;
  std::vector<uint32_t> m_status_ids;
};

namespace std {
  template <>
  struct hash<sf::Vector3i> {
    std::size_t operator()(const sf::Vector3i& tile) const {
      // Arbitrarily large prime numbers
      const size_t h1 = 0x8da6b343;
      const size_t h2 = 0xd8163841;
      const size_t h3 = 0xcb1ab31f;
      return tile.x * h1 + tile.y * h2 + tile.z * h3;
    }
  };
}
