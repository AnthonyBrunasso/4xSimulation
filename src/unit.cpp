#include "unit.h"

#include "unique_id.h"
#include "tile.h"
#include "format.h"
#include "hex.h"
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

uint32_t unit::create(UNIT_TYPE unit_type, const sf::Vector3i& location, uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) {
    return 0;
  }

  uint32_t id = unique_id::generate();
  Unit* unit = new Unit(id, unit_type);
  unit->m_location = location;
  unit->m_owner_id = player_id;
  
  // Apply unit specific stats if they exist.
  CombatStats* stats = unit_definitions::get(unit_type);
  if (stats) {
    unit->m_combat_stats = *stats;
  }

  // Add the unit to storage and the world map.
  s_units[id] = unit;
  player::add_unit(player_id, id);
  std::cout << "Created unit id " << id << ", entity type: " << get_unit_name(unit_type) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void unit::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

bool unit::destroy(uint32_t id) {
  Unit* unit = get_unit(id);
  if (!unit) {
    return false;
  }

  // Notify all subscribers of unit death with its location and unique id
  for (auto sub : s_destroy_subs) {
    sub(unit->m_location, id);
  }

  s_units.erase(unit->m_id);
  delete unit;
  return true;
}

void unit::sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_destroy_subs.push_back(sub);
}

Unit* unit::get_unit(uint32_t id) {
  if (s_units.find(id) == s_units.end()) {
    return nullptr;
  }

  return s_units[id];
}

void unit::for_each_unit(std::function<void(const Unit& unit)> operation) {
  for (auto unit : s_units) {
    operation(*unit.second);
  }
}

void unit::set_path(uint32_t id, const std::vector<sf::Vector3i>& path) {
  Unit* unit = get_unit(id);
  if (!unit) {
    return;
  }

  unit->m_path = path;
  std::cout << "Path size: " << unit->m_path.size() << std::endl;
}

void unit::replenish_actions() {
  std::cout << "Replentish action points" << std::endl;
  for (auto unit : s_units) {
    unit.second->m_action_points = unit.second->m_max_actions;
  }
}

bool unit::combat(uint32_t attacker_id, uint32_t defender_id) {
  Unit* attacker = get_unit(attacker_id);
  Unit* defender = get_unit(defender_id);

  if (!attacker || !defender) {
    return false;
  }
  
  Modifier attack_mod, defend_mod;
  combat::calculate_modifiers(attacker, defender, attack_mod, defend_mod);

  std::cout << "Unit " << attacker_id << " vs. Unit " << defender_id << std::endl;

  // Get distance between characters
  uint32_t distance = hex::cube_distance(attacker->m_location, defender->m_location);
  // Engage in combat with no modifiers, will have to add some logic to come up with modifiers here
  bool result = combat::engage(
    attacker->m_combat_stats, 
    attack_mod, 
    defender->m_combat_stats, 
    defend_mod, 
    distance);

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

bool unit::damage(uint32_t receiver_id, float amount) {
  Unit* receiver = get_unit(receiver_id);
  if(!receiver) return false;

  float damage_delt = std::min(amount, receiver->m_combat_stats.m_health);
  receiver->m_combat_stats.m_health -= damage_delt;
  if (receiver->m_combat_stats.m_health == 0) {
    std::cout << "Unit " << receiver_id << " received lethal damage." << std::endl;
    return destroy(receiver_id);
  }

  return false;
}

void unit::heal(uint32_t receiver_id, float amount) {
  Unit* receiver = get_unit(receiver_id);
  if(!receiver) return;
  CombatStats* stats = unit_definitions::get(receiver->m_unit_type);
  if (!stats) return;
  float new_health = std::min(receiver->m_combat_stats.m_health+amount, stats->m_health);
  receiver->m_combat_stats.m_health = new_health;
}

void unit::change_direction(uint32_t id, const sf::Vector3i& target) {
  Unit* u = get_unit(id);
  if (!u) return;
  u->m_direction = target - u->m_location;
}

void unit::clear() {
  for (auto unit : s_units) {
    delete unit.second;
  }
  
  s_units.clear();
}
