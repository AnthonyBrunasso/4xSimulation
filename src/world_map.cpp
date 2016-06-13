#include "world_map.h"

#include "search.h"
#include "tile.h"
#include "units.h"
#include "city.h"
#include "unique_id.h"
#include "format.h"
#include "improvements.h"
#include "tile_costs.h"
#include "player.h"
#include "ai_barbarians.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>

namespace {
  static world_map::TileMap s_map;
  static std::vector<sf::Vector3i> s_coords;
  
  void subscribe_to_events();
  void set_improvement_requirements();
  void set_city_requirements();

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

    city::sub_raze([](const sf::Vector3i& location, uint32_t /*id*/) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        return;
      }
      
      tile->m_city_id = unique_id::INVALID_ID;
    });

    improvement::sub_create([](const sf::Vector3i& location, uint32_t id) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        return;
      }
      tile->m_improvement_ids.push_back(id);
    });

    improvement::sub_destroy([](const sf::Vector3i& location, uint32_t id) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        return;
      }
      auto findIt = std::find(tile->m_improvement_ids.begin(), tile->m_improvement_ids.end(), id);
      if (findIt != tile->m_improvement_ids.end()) {
        tile->m_improvement_ids.erase(findIt);
      }     
    });
  }

  void set_improvement_requirements() {
    improvement::add_requirement(IMPROVEMENT_TYPE::RESOURCE, [](const sf::Vector3i& location) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        std::cout << "Tile does not exist at location: " << format::vector3(location) << std::endl;
        return false;
      }

      if (tile->m_resources.empty()) {
        std::cout << "No resources exist on tile: " << format::vector3(location) << std::endl;
        return false;
      }

      // Check if this tile already contains a resource improvement.
      for (auto id : tile->m_improvement_ids) {
        Improvement* improvement = improvement::get_improvement(id);
        if (!improvement) continue;
        if (improvement->m_type == IMPROVEMENT_TYPE::RESOURCE) {
          std::cout << "Resource improvement already exists on this tile" << std::endl;
          return false;
        }
      }

      return true;
    });
  }

  void set_city_requirements() {
    city::add_requirement(BUILDING_TYPE::TOWN, [](const sf::Vector3i& location, uint32_t player_id) {
      Tile* tile = world_map::get_tile(location);
      if (!tile) {
        std::cout << "Tile does not exist at location: " << format::vector3(location) << std::endl;
        return false;
      }

      Player* player = player::get_player(player_id);
      if (!player) {
        std::cout << "Invalid player id: " << player_id << std::endl;
        return false;
      }

      // Check if this tile already contains a resource improvement.
      for (auto id : tile->m_unit_ids) {
        Unit* unit = units::get_unit(id);
        if (!unit) continue;
        if (unit->m_unit_type == UNIT_TYPE::WORKER) {
          // Get the player and check that the player owns this unit.
          if (player->OwnsUnit(unit->m_unique_id)) {
            return true;
          }
        }
      }
      
      // No worker is contained on the tile.
      std::cout << player->m_name << " does not own a worker at " << format::vector3(location) << std::endl;
      return false;
    });
  }
}

void world_map::build(sf::Vector3i start, uint32_t size) {
  search::range(start, size, s_coords);
  for (auto tile : s_coords) {
    s_map[tile] = Tile();
  }

  tile_costs::initialize();
  barbarians::initialize();

  subscribe_to_events();
  set_improvement_requirements();
  set_city_requirements();
}

bool world_map::load_file(const std::string& name) {
  std::ifstream inputFile(name.c_str(), std::ios::binary | std::ios::in);
  const size_t BLOCK_SIZE = 4;
  char data[BLOCK_SIZE];

  if (!inputFile.good()) {
    std::cout << "file is not good " << name << std::endl;
    return false;
  }

  for (auto& tile : s_map) {
    if (!inputFile.good()) {
      std::cout << "Bailed on map read, file data < map tile count" << std::endl;
      return false;
    }

    memset(data, 0, sizeof(data));
    inputFile.read(data, BLOCK_SIZE);
    TERRAIN_TYPE terrain_type = static_cast<TERRAIN_TYPE>(*data);
    tile.second.m_terrain_type = terrain_type;
    tile.second.m_path_cost = tile_costs::get(terrain_type);
  }

  // read eof
  inputFile.read(data, 1);
  // If we have data unread, the world is of a mismatched size
  if (inputFile.good()) {
    std::cout << "Bailed on map read, file data > map tile count" << std::endl;
    return false;
  }

  return true;
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
  return true;
}

uint32_t world_map::move_unit(uint32_t unit_id, uint32_t distance) {
  Unit* unit = units::get_unit(unit_id);
  if (!unit) {
    return 0;
  }

  uint32_t moved = 0;
  for (uint32_t i = 0; i < distance; ++i) {
    // First item in list is up next 
    Tile* next = get_tile(unit->m_path[0]);
    // Early out if invalid tile
    if (!next) {
      return moved;
    }
    // Remove unit from it's current standing place
    remove_unit(unit->m_location, unit->m_unique_id);
    // Move it to new tile
    std::cout << "Unit " << unit->m_unique_id << " (id) moved from: " << format::vector3(unit->m_location) << " to: " << format::vector3(unit->m_path[0]) << std::endl;
    unit->m_location = unit->m_path[0];
    next->m_unit_ids.push_back(unit->m_unique_id);
    // Remove tile moved to, always erasing first TODO: Fix that when pathing implemented
    unit->m_path.erase(unit->m_path.begin());
    ++moved;
  }

  return moved;
}

world_map::TileMap& world_map::get_map() {
  return s_map;
}

Tile* world_map::get_tile(sf::Vector3i location) {
  if (s_map.find(location) == s_map.end()) {
    return nullptr;
  }

  return &s_map[location];
}
