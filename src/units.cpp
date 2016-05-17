#include "units.h"

#include "unique_id.h"
#include "world_map.h"
#include "tile.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace {
  typedef std::unordered_map<uint32_t, Unit*> UnitMap;
  UnitMap s_units;
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

  Tile* tile = world_map::get_tile(unit->m_location);
  if (tile) {
    auto findIt = std::find(tile->m_unit_ids.begin(), tile->m_unit_ids.end(), unit->m_unique_id);
    if (findIt != tile->m_unit_ids.end()) {
      tile->m_unit_ids.erase(findIt);
    }
  }

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

void units::clear() {
  for (auto unit : s_units) {
    delete unit.second;
  }
  s_units.clear();
}