#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "Vector3.hpp"
#include "world_map.h"

class City;
class Tile;
class Unit;
struct Improvement;
struct Resource;

namespace search {
  std::vector<sf::Vector3i> range(const sf::Vector3i& start, int32_t distance);
  std::vector<sf::Vector3i> path_to(const sf::Vector3i& start,
      const sf::Vector3i& end, 
      world_map::TileMap& tile_map,
      std::function<bool(const Tile& tile)> expand = {});

  // NOTE:
  // For bfs search, 
  //   if the comparator returns true the bfs will stop expanding at the first found instance.
  //   if the comparator returns false it will continue to expand all nodes.
  // Therefore, return true if you want to find a single instance of a search, false otherwise.

  // Returns true if any tile within the depth of a breadth first search meets the
  // criteria given by the comparator.
  bool bfs(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const Tile& tile)> comparator);

  // Run bfs for each unit to depth.
  bool bfs_units(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const Unit& )> comparator);

  // Run bfs for each city to depth.
  bool bfs_cities(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const City& )> comparator);

  // Run bfs for each improvement to depth.
  bool bfs_improvements(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const Improvement& )> comparator);

  // Run bfs for each resource to depth.
  bool bfs_resources(const sf::Vector3i& start,
      uint32_t depth,
      world_map::TileMap& tile_map,
      std::function<bool(const Resource& )> comparator);

}
