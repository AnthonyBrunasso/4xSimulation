#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <vector>

// Tiles contain -
// * unique id : to identify any improvements on the tile
// * terrain id: to identify any special geogrpahic properties of the tile
// * array of entity ids: to identify buildings, units, etc that are contained on the tile
class Tile {
public:
  Tile();

  uint32_t m_unique_id;
  uint32_t m_terrain_id;
  std::vector<uint32_t> m_entity_ids;
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