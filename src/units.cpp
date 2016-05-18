#include "units.h"

#include "unique_id.h"
#include "world_map.h"
#include "tile.h"
#include "format.h"
#include "hex.h"
#include "util.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <vector>

namespace {
  typedef std::unordered_map<uint32_t, Unit*> UnitMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  UnitMap s_units;
  SubMap s_destroy_subs;
  SubMap s_create_subs;
}

uint32_t units::create(ENTITY_TYPE entity_type, const sf::Vector3i& location) {
  uint32_t id = unique_id::generate();
  Unit* unit = new Unit(id, entity_type);
  unit->m_location = location;

  // Add the unit to storage and the world map
  s_units[id] = unit;
  std::cout << "Created unit id " << id << ", entity type: " << static_cast<uint32_t>(entity_type) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void units::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void units::destroy(uint32_t id) {
  Unit* unit = s_units[id];
  if (!unit) {
    return;
  }

  // Notify all subscribers of unit death with its location and unique id
  for (auto sub : s_destroy_subs) {
    sub(unit->m_location, id);
  }

  s_units.erase(unit->m_unique_id);
  delete unit;
}

void units::sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_destroy_subs.push_back(sub);
}

Unit* units::get_unit(uint32_t id) {
  if (s_units.find(id) == s_units.end()) {
    return nullptr;
  }

  return s_units[id];
}

void units::for_each_unit(std::function<void(const Unit& unit)> operation) {
  for (auto unit : s_units) {
    operation(*unit.second);
  }
}

void units::set_path(uint32_t id, const sf::Vector3i& destination) {
  Unit* unit = get_unit(id);
  if (!unit) {
    return;
  }

  // Set the destination of the unit and queue it to move
  // TODO: Change this to a search algorithm like A* that weights tiles, for now, assume all tiles can be pathed
  hex::cubes_on_line(util::to_vector3f(unit->m_location), 
    util::to_vector3f(destination), 
    unit->m_path);

  std::cout << "Path size: " << unit->m_path.size() << std::endl;

  // TODO: When pathing algorithm built deal with not removing the first element on each move
  if (unit->m_path.size()) {
    unit->m_path.erase(unit->m_path.begin());
  }
}

uint32_t units::move(uint32_t id, uint32_t distance) {
  Unit* unit = get_unit(id);
  if (!unit) {
    return 0;
  }

  uint32_t moved = 0;
  for (uint32_t i = 0; i < distance; ++i) {
    // First item in list is up next 
    Tile* next = world_map::get_tile(unit->m_path[0]);
    // Early out if invalid tile
    if (!next) {
      return moved;
    }
    // Remove unit from it's current standing place
    world_map::remove_unit(unit->m_location, unit->m_unique_id);
    // Move it to new tile
    std::cout << "Unit moved from: " << format::vector3(unit->m_location) << " to: " << format::vector3(unit->m_path[0]) << std::endl;
    unit->m_location = unit->m_path[0];
    next->m_unit_ids.push_back(unit->m_unique_id);
    // Remove tile moved to, always erasing first TODO: Fix that when pathing implemented
    unit->m_path.erase(unit->m_path.begin());
    ++moved;
  }

  return moved;
}

void units::replenish_actions() {
  for (auto unit : s_units) {
    unit.second->m_action_points = unit.second->m_max_actions;
  }
}

void units::clear() {
  for (auto unit : s_units) {
    delete unit.second;
  }
  
  s_units.clear();
}