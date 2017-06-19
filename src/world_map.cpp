#include "world_map.h"

//#include <ext/alloc_traits.h>
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
#include "improvement.h"
#include "map_generated.h"
#include "player.h"
#include "random.h"
#include "resources.h"
#include "search.h"
#include "step_generated.h"
#include "tile.h"
#include "tile_costs.h"
#include "unique_id.h"
#include "unit.h"
#include "util.h"


#include "flatbuffers/util.h"

namespace world_map {
  static TileMap s_map;
  static uint32_t s_map_size;
  
  void subscribe_to_events();

  void unit_create(Unit* u) {
    world_map::add_unit(u->m_location, u->m_id);
  } 

  void unit_destroy(UnitFatality* uf) {
    world_map::remove_unit(uf->m_dead->m_location, uf->m_dead->m_id);
  }

  bool city_create(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return false;
    }
    tile->m_city_id = id;
    return true;
  }

  bool city_raze(const sf::Vector3i& location, uint32_t /*id*/) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return false;
    }
    
    tile->m_city_id = unique_id::INVALID_ID;
    return true;
  }

  bool improvement_create(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return false;
    }
    tile->m_improvement_ids.push_back(id);
    return true;
  }

  bool improvement_destroy(const sf::Vector3i& location, uint32_t id) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      return false;
    }
    auto findIt = std::find(tile->m_improvement_ids.begin(), tile->m_improvement_ids.end(), id);
    if (findIt != tile->m_improvement_ids.end()) {
      tile->m_improvement_ids.erase(findIt);
    }     
    return true;
  }

  void subscribe_to_events() {
    unit::sub_create(unit_create);
    city::sub_create(city_create);
    unit::sub_destroy(unit_destroy);
    city::sub_raze_complete(city_raze);
    improvement::sub_create(improvement_create);
    improvement::sub_destroy(improvement_destroy);
  }

  flatbuffers::FlatBufferBuilder& GetFBB() {
    static flatbuffers::FlatBufferBuilder builder;
    return builder;
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
}

bool world_map::load_file_fb(const std::string& name) {
  std::string map_data;
  if (!flatbuffers::LoadFile(name.c_str(), true, &map_data)) return false;

  flatbuffers::Verifier v(reinterpret_cast<const uint8_t*>(map_data.c_str()), map_data.size());
  if (!fbs::VerifyMapBuffer(v)) return false;

  auto root = fbs::GetMap(map_data.c_str());
  //todo set map size
  std::vector<sf::Vector3i> coords;
  coords = search::range(sf::Vector3i(0, 0, 0), s_map_size);
  std::cout << "Loading map of size " << s_map_size << " with " << coords.size() << " tiles." << std::endl;

  const flatbuffers::Vector<uint32_t> *terrain = root->terrain();
  if (terrain->size() != coords.size()) return false;

  for (size_t i = 0; i < coords.size(); ++i) {
    const auto& coord = coords[i];
    auto& tile = s_map[coord];

    uint32_t tval = (*terrain)[i];
    fbs::TERRAIN_TYPE terrain_type = any_enum(tval);
    tile.m_terrain_type = terrain_type;
    tile.m_path_cost = tile_costs::get(terrain_type);
  }
  std::cout << "Terrain read complete." << std::endl;

  const flatbuffers::Vector<uint32_t> *resource = root->resource();
  if (resource->size() != coords.size()) return false;
  for (size_t i = 0; i < coords.size(); ++i) {
    const auto& coord = coords[i];
    auto& tile = s_map[coord];
    uint32_t rval = (*resource)[i];
    fbs::RESOURCE_TYPE resource_type = any_enum(rval);

    Resource r;
    r.m_type = resource_type; // may be RESOURCE_TYPE::UNKNOWN (none)
    r.m_quantity = 1;
    tile.m_resource = r;
  }
  std::cout << "Resource read complete." << std::endl;

  return true;
}

bool world_map::save_file_fb(const char* name) {
  std::vector<sf::Vector3i> coords;
  coords = search::range(sf::Vector3i(0, 0, 0), s_map_size);

  std::cout << "Saving map of size " << s_map_size << " with " << coords.size() << " tiles." << std::endl;
  std::vector<uint32_t> terrain(coords.size());
  std::vector<uint32_t> resource(coords.size());

  for (size_t i = 0; i < coords.size(); ++i) {
    auto& tile = s_map[coords[i]];
    fbs::TERRAIN_TYPE tval = tile.m_terrain_type;
    fbs::RESOURCE_TYPE rval = tile.m_resource.m_type;
    terrain[i] = any_enum(tval);
    resource[i] = any_enum(rval);
  }

  const auto map_data = fbs::CreateMap(GetFBB(), s_map_size,
      GetFBB().CreateVector(terrain),
      GetFBB().CreateVector(resource));
  fbs::FinishMapBuffer(GetFBB(), map_data);
  if (!flatbuffers::SaveFile(name, reinterpret_cast<const char*>(GetFBB().GetBufferPointer()), GetFBB().GetSize(), true)) return false;
  std::cout << "Map save completed." << std::endl;

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
