#include "unit.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "combat.h"
#include "player.h"
#include "step_generated.h"
#include "unique_id.h"
#include "unit_definitions.h"

namespace {
  typedef std::unordered_map<uint32_t, Unit*> UnitMap;
  typedef std::vector<std::function<void(Unit*)> > SubMap;
  typedef std::vector<std::function<void(UnitFatality*)> > DestroySubMap;
  UnitMap s_units;
  DestroySubMap s_destroy_subs;
  SubMap s_create_subs;
}

Unit::Unit() : m_type(fbs::UNIT_TYPE::UNKNOWN)
, m_id(unique_id::INVALID_ID)
, m_location()
, m_path()
, m_action_points(0)
, m_owner_id(unique_id::INVALID_PLAYER)
, m_direction(0, 0, 0)
{
}

Unit::Unit(uint32_t unique_id) 
: m_type(fbs::UNIT_TYPE::UNKNOWN)
, m_id(unique_id)
, m_location()
, m_path()
, m_action_points(0)
, m_owner_id(unique_id::INVALID_PLAYER)
, m_direction(0, 0, 0)
{
}

uint32_t unit::create(fbs::UNIT_TYPE unit_type, const sf::Vector3i& location, uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) {
    return 0;
  }

  uint32_t id = unique_id::generate();
  Unit* unit = new Unit(id);
  unit->m_location = location;
  unit->m_owner_id = player_id;
  unit->m_type = unit_type;
  
  // Apply unit specific stats if they exist.
  CombatStats* stats = unit_definitions::get(unit_type);
  if (stats) {
    unit->m_combat_stats = *stats;
  }
  unit->m_action_points = unit->m_combat_stats.m_action_points;

  // Add the unit to storage and the world map.
  s_units[id] = unit;
  player::add_unit(player_id, id);
  std::cout << "Created unit id " << id << ", entity type: " << fbs::EnumNameUNIT_TYPE(unit_type) << std::endl;

  for (auto& sub : s_create_subs) {
    sub(unit);
  }

  return id;
}

void unit::sub_create(std::function<void(Unit*)> sub) {
  s_create_subs.push_back(sub);
}

bool unit::destroy(uint32_t dead_id, uint32_t attacking_id, uint32_t opponent_id)
{
  Unit* unit = get_unit(dead_id);
  if (!unit) {
    return false;
  }

  UnitFatality uf(unit);
  uf.m_opponent = player::get_player(opponent_id);
  uf.m_attacking = unit::get_unit(attacking_id);

  // Notify all subscribers of unit death with its location and unique id
  for (auto& sub : s_destroy_subs) {
    sub(&uf);
  }

  s_units.erase(unit->m_id);
  delete unit;
  return true;
}

void unit::sub_destroy(std::function<void(UnitFatality*)> sub) {
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
  for (auto& member : s_units) {
    Unit* unit = member.second;
    CombatStats* stats = unit_definitions::get(unit->m_type);
    if (!stats) return;
    unit->m_action_points = stats->m_action_points;
  }
}

bool unit::combat(uint32_t attacker_id, uint32_t defender_id) {
  Unit* attacker = get_unit(attacker_id);
  Unit* defender = get_unit(defender_id);

  if (!attacker || !defender) {
    return false;
  }
  
  std::cout << "--Unit " << attacker_id << " vs. Unit " << defender_id << std::endl;

  // Engage in combat with no modifiers, will have to add some logic to come up with modifiers here
  bool result = combat::engage(
    attacker, 
    defender);

  // If attacker or defender died, kill them
  if (defender->m_combat_stats.m_health == 0) {
    std::cout << "defending unit " << defender_id << " (id) destroyed in combat." << std::endl;
    destroy(defender_id, attacker_id, attacker->m_owner_id);
  }

  if (attacker->m_combat_stats.m_health == 0) {
    std::cout << "attacking unit " << attacker_id << " (id) destroyed in combat." << std::endl;
    destroy(attacker_id, defender_id, defender->m_owner_id);
  }

  return result;
}

bool unit::damage(uint32_t receiver_id, uint32_t source_player, float amount) {
  Unit* receiver = get_unit(receiver_id);
  if(!receiver) return false;

  Player* source = player::get_player(source_player);
  if (!source) return false;

  float damage_delt = std::min(amount, receiver->m_combat_stats.m_health);
  receiver->m_combat_stats.m_health -= damage_delt;
  if (receiver->m_combat_stats.m_health == 0) {
    std::cout << "Unit " << receiver_id << " received lethal damage." << std::endl;
    return destroy(receiver_id, unique_id::INVALID_ID, source_player);
  }

  return false;
}

void unit::heal(uint32_t receiver_id, float amount) {
  Unit* receiver = get_unit(receiver_id);
  if(!receiver) return;
  CombatStats* stats = unit_definitions::get(receiver->m_type);
  if (!stats) return;
  float new_health = std::min(receiver->m_combat_stats.m_health+amount, stats->m_health);
  receiver->m_combat_stats.m_health = new_health;
}

void unit::change_direction(uint32_t id, const sf::Vector3i& target) {
  Unit* u = get_unit(id);
  if (!u) return;
  u->m_direction = target - u->m_location;
}

size_t unit::size() {
  return s_units.size();
}

void unit::reset() {
  for (auto& unit : s_units) {
    delete unit.second;
  }
  
  s_units.clear();
  s_destroy_subs.clear();
  s_create_subs.clear();
}
