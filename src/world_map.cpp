#include "world_map.h"

#include <ext/alloc_traits.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "ai_barbarians.h"
#include "city.h"
#include "format.h"
#include "game_types.h"
#include "improvement.h"
#include "player.h"
#include "random.h"
#include "resources.h"
#include "search.h"
#include "tile.h"
#include "tile_costs.h"
#include "unique_id.h"
#include "unit.h"

namespace {
  static world_map::TileMap s_map;
  static uint32_t s_map_size;
  
  void subscribe_to_events();
  void set_improvement_requirements();
  void set_city_requirements();

  void unit_create(Unit* u) {
    world_map::add_unit(u->m_location, u->m_id);
  } 

  void unit_destroy(UnitFatality* uf) {
    world_map::remove_unit(uf->m_dead->m_location, uf->m_dead->m_id);
  }

  void city_create(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return;
    }
    tile->m_city_id = id;
  }

  void city_raze(const sf::Vector3i& location, uint32_t /*id*/) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return;
    }
    
    tile->m_city_id = unique_id::INVALID_ID;
  }

  void improvement_create(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return;
    }
    tile->m_improvement_ids.push_back(id);
  }

  void improvement_destroy(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return;
    }
    auto findIt = std::find(tile->m_improvement_ids.begin(), tile->m_improvement_ids.end(), id);
    if (findIt != tile->m_improvement_ids.end()) {
      tile->m_improvement_ids.erase(findIt);
    }     
  }

  bool is_resource_available(RESOURCE_TYPE rt, IMPROVEMENT_TYPE type, const sf::Vector3i& location) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return false;
    }
    
    // Check if this tile already contains a resource improvement.
    for (auto id : tile->m_improvement_ids) {
      Improvement* improvement = improvement::get_improvement(id);
      if (!improvement) continue;
      if (improvement->m_resource.m_type == rt) {
        std::cout << "Resource improvement already exists on this tile" << std::endl;
        return false;
      }
    }

    return true;
  }

  bool valid_resource(RESOURCE_TYPE selected_type
      , IMPROVEMENT_TYPE type
      , const sf::Vector3i& location) {
    return type == improvement::resource_improvement(selected_type);
  }

  void set_improvement_requirements() {
    for_each_improvement_type([] (IMPROVEMENT_TYPE impv) {
      improvement::add_requirement(impv, is_resource_available);
      improvement::add_requirement(impv, valid_resource);
    });
  }

  bool town_requirement(const sf::Vector3i& location, uint32_t player_id) {
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
      Unit* unit = unit::get_unit(id);
      if (!unit) continue;
      if (unit->m_type == UNIT_TYPE::WORKER) {
        // Get the player and check that the player owns this unit.
        if (player->OwnsUnit(unit->m_id)) {
          return true;
        }
      }
    }
    
    // No worker is contained on the tile.
    std::cout << player->m_name << " does not own a worker at " << format::vector3(location) << std::endl;
    return false;
  }

  void subscribe_to_events() {
    unit::sub_create(unit_create);
    city::sub_create(city_create);
    unit::sub_destroy(unit_destroy);
    city::sub_raze_complete(city_raze);
    improvement::sub_create(improvement_create);
    improvement::sub_destroy(improvement_destroy);
  }

  void set_city_requirements() {
    city::add_requirement(BUILDING_TYPE::TOWN, town_requirement);
  }
}

void init_discoverable_tiles(uint32_t size) {
  uint32_t tiles = size / 2;
  for (uint32_t i = 0; i < tiles; ++i) {
    sf::Vector3i cube = game_random::cube_coord(size);
    Tile* tile = world_map::get_tile(cube);
    if (!tile) {
      std::cout << "Failed to place discoverable bonus " << format::vector3(cube) << std::endl;
      continue;
    }
    tile->m_discover_bonus = true;
  }
}

void world_map::build(sf::Vector3i start, uint32_t size) {
  std::vector<sf::Vector3i> coords = search::range(start, size);
  for (auto tile : coords) {
    s_map[tile] = Tile(tile);
  }
  s_map_size = size;
  init_discoverable_tiles(size);

  tile_costs::initialize();
  barbarians::initialize();
  improvement::initialize();

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

  std::vector<sf::Vector3i> coords;
  coords = search::range(sf::Vector3i(0, 0, 0), s_map_size);
  for (const auto& coord : coords) {
    auto& tile = s_map[coord];
    if (!inputFile.good()) {
      std::cout << "map tiles, file data < map tile count" << std::endl;
      return false;
    }

    memset(data, 0, sizeof(data));
    inputFile.read(data, BLOCK_SIZE);

    TERRAIN_TYPE terrain_type = static_cast<TERRAIN_TYPE>(*data);
    tile.m_terrain_type = terrain_type;
    tile.m_path_cost = tile_costs::get(terrain_type);
  }

  // read seperator
  inputFile.read(data, BLOCK_SIZE);

  // process any map resources
  for (const auto& coord : coords) {
    auto& tile = s_map[coord];
    if (!inputFile.good()) {
      std::cout << "map resources, file data < map tile count" << std::endl;
      return false;
    }

    memset(data, 0, sizeof(data));
    inputFile.read(data, BLOCK_SIZE);

    RESOURCE_TYPE resource_type = static_cast<RESOURCE_TYPE>(*data);
    if (resource_type == RESOURCE_TYPE::UNKNOWN) continue;

    tile.m_resources.push_back(Resource(resource_type));
  }

  // read seperator
  inputFile.read(data, BLOCK_SIZE);

  // read eof
  inputFile.read(data, 1);
  // If we have data unread, the world is of a mismatched size
  if (inputFile.good()) {
    std::cout << "Bailed on map read, file data > map tile count" << std::endl;
    return false;
  }

  return true;
}

bool world_map::write_file(const char* name) {
  std::ofstream out(name, std::ios::out);
  const size_t BLOCK_SIZE = 4;
  char seperator[] = { 0,0,0,0 };

  std::vector<sf::Vector3i> coords;
  coords = search::range(sf::Vector3i(0, 0, 0), s_map_size);
  for (auto& coord : coords) {
    auto& tile = s_map[coord];
    uint32_t type = static_cast<uint32_t>(tile.m_terrain_type);
    out.write(reinterpret_cast<const char*>(&type), BLOCK_SIZE);
  }

  out.write(seperator, BLOCK_SIZE);

  for (auto& coord : coords) {
    auto& tile = s_map[coord];
    uint32_t resource_type = 0;
    if (tile.m_resources.size()) {
      resource_type = static_cast<uint32_t>(tile.m_resources.front().m_type);
    }

    out.write(reinterpret_cast<const char*>(&resource_type), BLOCK_SIZE);
  }

  out.write(seperator, BLOCK_SIZE);

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
  Unit* unit = unit::get_unit(unit_id);
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
    remove_unit(unit->m_location, unit->m_id);
    // Move it to new tile
    std::cout << "Unit " << unit->m_id << " (id) moved from: " << format::vector3(unit->m_location) << " to: " << format::vector3(unit->m_path[0]) << std::endl;
    sf::Vector3i difference = unit->m_path[0] - unit->m_location;
    unit->m_direction = difference;
    unit->m_location = unit->m_path[0];
    Player* player = player::get_player(unit->m_owner_id);
    if (next->m_discover_bonus) {
      next->m_discover_bonus = false;
      std::cout << " Unit " << unit->m_id << " discovered a magical relic! Your civilization gains 50 gold." << std::endl;
      if (player) {
        player->m_gold += 50.f;
      }
    }
    if (player) {
      auto fp = [player](const Tile& t) -> bool {
        player->m_discovered_tiles.insert(&t);
        return false;
      };
      search::bfs(unit->m_location, 2, world_map::get_map(), fp);
    }
    next->m_unit_ids.push_back(unit->m_id);
    // Remove tile moved to, always erasing first TODO: Fix that when pathing implemented
    unit->m_path.erase(unit->m_path.begin());
    ++moved;
  }

  return moved;
}

world_map::TileMap& world_map::get_map() {
  return s_map;
}

uint32_t world_map::get_map_size() {
  return s_map_size;
}

Tile* world_map::get_tile(sf::Vector3i location) {
  if (s_map.find(location) == s_map.end()) {
    return nullptr;
  }

  return &s_map[location];
}

uint32_t world_map::tile_owner(const Tile& tile) {
  City* city = city::get_city(tile.m_city_id);
  if (city) {
    return city->m_owner_id;
  }

  for (size_t i = 0; i < tile.m_unit_ids.size(); ++i) {
    Unit* u = unit::get_unit(tile.m_unit_ids[i]);
    if (u) {
      return u->m_owner_id;
    }
  }

  return unique_id::INVALID_PLAYER;
}

void world_map::reset() {
  s_map.clear();
  s_map_size = 0;
}
