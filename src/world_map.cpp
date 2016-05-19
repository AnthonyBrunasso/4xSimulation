#include "world_map.h"

#include "search.h"
#include "tile.h"
#include "units.h"
#include "city.h"

#include <unordered_map>
#include <vector>

namespace {
  static std::unordered_map<sf::Vector3i, Tile> s_map;
  static std::vector<sf::Vector3i> s_coords;

  void subscribe_to_events();

  void subscribe_to_events() {
    units::sub_create([](const sf::Vector3i& location, uint32_t id) {
      world_map::add_unit(location, id);
    });

    city::sub_create([](const sf::Vector3i& location, uint32_t id) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        return;
      }
      tile->m_city_id = id;
    });

    units::sub_destroy([](const sf::Vector3i& location, uint32_t id) {
      world_map::remove_unit(location, id);
    });

    city::sub_raze([](const sf::Vector3i& location, uint32_t id) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        return;
      }
      tile->m_city_id = unique_id::INVALID_ID;
    });
  }
}

void world_map::build(sf::Vector3i start, uint32_t size) {
  search::range(start, size, s_coords);
  for (auto tile : s_coords) {
    s_map[tile] = Tile();
  }

  subscribe_to_events();  
}

void world_map::for_each_tile(std::function<void(const sf::Vector3i& coord, const Tile& tile)> operation) {
  for (auto tile : s_map) {
    operation(tile.first, tile.second);
  }
}

bool world_map::remove_unit(const sf::Vector3i& location, uint32_t unit_id) {
  Tile* tile = world_map::get_tile(location);
  if (tile) {
    auto findIt = std::find(tile->m_unit_ids.begin(), tile->m_unit_ids.end(), unit_id);
    if (findIt != tile->m_unit_ids.end()) {
      tile->m_unit_ids.erase(findIt);
      return true;
    }
  }
  return false;
}

bool world_map::add_unit(const sf::Vector3i& location, uint32_t unit_id) {
  Tile* tile = world_map::get_tile(location);
  if (!tile) {
    return false;
  }
  tile->m_unit_ids.push_back(unit_id);
}

Tile* world_map::get_tile(sf::Vector3i location) {
  if (s_map.find(location) == s_map.end()) {
    return nullptr;
  }

  return &s_map[location];
}