#include "units.h"

#include "unique_id.h"
#include "world_map.h"
#include "tile.h"
#include "format.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace {
  typedef std::unordered_map<uint32_t, Unit*> UnitMap;
  UnitMap s_units;

  bool remove_from_tile(const sf::Vector3i& location, uint32_t unit_id);

  bool remove_from_tile(const sf::Vector3i& location, uint32_t unit_id) {
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
}

uint32_t units::create(ENTITY_TYPE entity_type, const sf::Vector3i& location) {
  Tile* tile = world_map::get_tile(location);
  if (!tile) {
    return unique_id::INVALID_ID;
  }
  uint32_t id = unique_id::generate();
  Unit* unit = new Unit(id, entity_type);
  unit->m_location = location;

  // Add the unit to storage and the world map
  s_units[id] = unit;
  std::cout << "Created unit id " << id << ", entity type: " << static_cast<uint32_t>(entity_type) << std::endl;
  tile->m_unit_ids.push_back(id);
  return id;
}

void units::destroy(uint32_t id) {
  Unit* unit = s_units[id];
  if (!unit) {
    return;
  }

  remove_from_tile(unit->m_location, unit->m_unique_id);
  s_units.erase(unit->m_unique_id);
  delete unit;
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
    remove_from_tile(unit->m_location, unit->m_unique_id);
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