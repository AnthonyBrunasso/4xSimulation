#include "units.h"

#include "world_map.h"
#include "tile.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace {
  static std::unordered_map<uint32_t, Unit*> s_units;
  static uint32_t s_unique_ids = 0;
  static const uint32_t INVALID_ID = UINT32_MAX;
}

uint32_t units::create(ENTITY_ID entity_id, const sf::Vector3i& location) {
  Tile* tile = world_map::get_tile(location);
  if (!tile) {
    return INVALID_ID;
  }
  Unit* unit = new Unit(s_unique_ids, entity_id);
  unit->m_location = location;

  // Add the unit to storage and the world map
  s_units[s_unique_ids] = unit;
  std::cout << "Created unit id " << s_unique_ids << ", entity type: " << static_cast<uint32_t>(entity_id) << std::endl;
  tile->m_occupied_ids.push_back(s_unique_ids);
  return s_unique_ids++;
}

void units::destroy(uint32_t id) {
  Unit* unit = s_units[id];
  if (!unit) {
    return;
  }

  Tile* tile = world_map::get_tile(unit->m_location);
  if (tile) {
    auto findIt = std::find(tile->m_occupied_ids.begin(), tile->m_occupied_ids.end(), unit->m_unique_id);
    if (findIt != tile->m_occupied_ids.end()) {
      tile->m_occupied_ids.erase(findIt);
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
