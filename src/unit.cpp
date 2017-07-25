#include "unit.h"

#include <string.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "combat.h"
#include "entity.h"
#include "enum_generated.h"
#include "player.h"
#include "unique_id.h"
#include "unit_definitions.h"
#include "util.h"

ECS_COMPONENT(Unit, 1023);

namespace unit {
  constexpr size_t SUBSCRIBER_LIMIT = 10;
  typedef std::function<void(Unit*)> UnitSubFunc;
  UnitSubFunc s_create_subs[SUBSCRIBER_LIMIT];
  typedef std::function<void(UnitFatality*)> UnitDeathFunc;
  UnitDeathFunc s_destroy_subs[SUBSCRIBER_LIMIT];

  constexpr size_t UNIT_NAME_MAX = 15;
  constexpr size_t UNIT_TYPE_LIMIT = (size_t)fbs::UNIT_TYPE::MAX+1;
  char s_unit_names[UNIT_TYPE_LIMIT][UNIT_NAME_MAX+1];

  const char* get_name(fbs::UNIT_TYPE);
}

const char* unit::get_name(fbs::UNIT_TYPE t) {
  uint32_t i = any_enum(t);
  if (strlen(s_unit_names[i])) return s_unit_names[i];
  return fbs::EnumNameUNIT_TYPE(t);
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
, m_name("")
, m_direction(0, 0, 0)
{
}

uint32_t unit::create(fbs::UNIT_TYPE unit_type, const sf::Vector3i& location, uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) {
    return 0;
  }

  uint32_t id = unique_id::generate();
  uint32_t c = create(id, s_Unit());
  Unit* unit = c_Unit(c);
  unit->m_id = id;
  unit->m_location = location;
  unit->m_owner_id = player_id;
  unit->m_type = unit_type;
  unit->m_name = get_name(unit_type);
  
  // Apply unit specific stats 
  CombatStats* stats = unit_definitions::get(unit_type);
  if (stats) {
    unit->m_combat_stats = *stats;
  }
  unit->m_food = unit->m_combat_stats.m_food;
  unit->m_action_points = unit->m_combat_stats.m_action_points;

  // Add the unit to storage and the world map.
  player::add_unit(player_id, id);
  std::cout << "Created unit id " << id << ", entity type: " << fbs::EnumNameUNIT_TYPE(unit_type) << std::endl;

  // Unit creation notify
  for (auto& sub : s_create_subs) {
    if (sub) {
      sub(unit);
    }
  }

  return id;
}

void unit::sub_create(std::function<void(Unit*)> sub) {
  for (auto &s : s_create_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
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
    if (sub) {
      sub(&uf);
    }
  }

  uint32_t c = delete_c(dead_id, s_Unit());
  std::cout << "Deleting entity " << dead_id << " at component " << c << std::endl;
  return true;
}

void unit::sub_destroy(std::function<void(UnitFatality*)> sub) {
  for (auto &s : s_destroy_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
}

Unit* unit::get_unit(uint32_t id) {
  uint32_t c = get(id, s_Unit());
  return c_Unit(c);
}

void unit::for_each_unit(std::function<void(const Unit& unit)> operation) {
  // TODO: revisit batch operations via ECS
  for (const auto& a : mapping_Unit) {
    if (a.entity == INVALID_ENTITY) continue;

    Unit* u = c_Unit(a.component);
    if (!u) continue;
    operation(*u);
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
  for (const auto& a : mapping_Unit) {
    if (a.entity == INVALID_ENTITY) continue;

    Unit* u = c_Unit(a.component);
    if (!u) continue;
    if (u->m_food == 0) continue;

    int usedAP = u->m_combat_stats.m_action_points-u->m_action_points;
    int usedFood = clamp(usedAP, 0, 1);
    u->m_food -= usedFood;
    u->m_action_points = u->m_combat_stats.m_action_points;
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
  size_t s = 0;
  for (const auto& a : mapping_Unit) {
    if (a.entity == INVALID_ENTITY) continue;
    ++s;
  }

  return s;
}

void unit::reset() {
  reset_ecs(s_Unit());

  for (char* name : s_unit_names) {
    *name = 0;
  }
  strncpy(s_unit_names[any_enum(fbs::UNIT_TYPE::WORKER)], "Bruce", UNIT_NAME_MAX);

  for (auto &s : s_destroy_subs) {
    s = UnitDeathFunc();
  }
  for (auto &s : s_create_subs) {
    s = UnitSubFunc();
  }
}
