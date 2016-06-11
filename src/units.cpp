#include "units.h"

#include "unique_id.h"
#include "tile.h"
#include "format.h"
#include "hex.h"
#include "util.h"
#include "combat.h"
#include "unit_definitions.h"

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

uint32_t units::create(UNIT_TYPE UNIT_TYPE, const sf::Vector3i& location) {
  uint32_t id = unique_id::generate();
  Unit* unit = new Unit(id, UNIT_TYPE);
  unit->m_location = location;

  // Apply unit specific stats if they exist.
  CombatStats* stats = unit_definitions::get(UNIT_TYPE);
  if (stats) {
    unit->m_combat_stats = *stats;
  }

  // Add the unit to storage and the world map.
  s_units[id] = unit;
  std::cout << "Created unit id " << id << ", entity type: " << static_cast<uint32_t>(UNIT_TYPE) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void units::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void units::destroy(uint32_t id) {
  Unit* unit = get_unit(id);
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

void units::replenish_actions() {
  std::cout << "Replentish action points" << std::endl;
  for (auto unit : s_units) {
    unit.second->m_action_points = unit.second->m_max_actions;
  }
}

bool units::combat(uint32_t attacker_id, uint32_t defender_id) {
  Unit* attacker = get_unit(attacker_id);
  Unit* defender = get_unit(defender_id);

  if (!attacker || !defender) {
    return false;
  }

  std::cout << "Unit " << attacker_id << " vs. Unit " << defender_id << std::endl;

  // Get distance between characters
  uint32_t distance = hex::cube_distance(attacker->m_location, defender->m_location);
  // Engage in combat with no modifiers, will have to add some logic to come up with modifiers here
  bool result = combat::engage(attacker->m_combat_stats, defender->m_combat_stats, distance);

  // If attacker or defender died, kill them
  if (defender->m_combat_stats.m_health == 0) {
    std::cout << "defending unit " << defender_id << " (id) destroyed in combat." << std::endl;
    destroy(defender_id);
  }

  if (attacker->m_combat_stats.m_health == 0) {
    std::cout << "attacking unit " << attacker_id << " (id) destroyed in combat." << std::endl;
    destroy(attacker_id);
  }

  return result;
}

void units::clear() {
  for (auto unit : s_units) {
    delete unit.second;
  }
  
  s_units.clear();
}
