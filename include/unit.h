#pragma once

#include "game_types.h"
#include "Vector3.hpp"
#include "combat.h"
#include "util.h"

#include <cstdint>
#include <functional>
#include <vector>

class Unit {
public:
  Unit(uint32_t unique_id, UNIT_TYPE UNIT_TYPE) 
    : m_unit_type(UNIT_TYPE)
    , m_id(unique_id)
    , m_location()
    , m_path()
    , m_max_actions(3)
    , m_action_points(m_max_actions)
    , m_owner_id(0xffffffff)
    // 1 health, 1 attack, 1 range by default
    , m_combat_stats(1, 1, 1)
    // Random start direction? 
    , m_direction(util::get_direction(DIRECTION_TYPE::EAST)){};

  UNIT_TYPE m_unit_type;
  uint32_t m_id;
  sf::Vector3i m_location;
  // Current path the unit is on, size is 0 if the unit is not moving
  std::vector<sf::Vector3i> m_path;
  // Number of actions available for the unit, simulation will dictate this value
  uint32_t m_max_actions;
  uint32_t m_action_points;
  uint32_t m_owner_id;

  CombatStats m_combat_stats;
  // Vector pointing the way the unit is facing. Directions in this game aren't normalized.
  sf::Vector3i m_direction;
};

namespace unit {
  uint32_t create(UNIT_TYPE unit_type, const sf::Vector3i& location, uint32_t player_id);
  // Subscribe to creation of a unit
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  bool destroy(uint32_t entity_id);
  // Subscribe to destruction of a unit
  void sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  Unit* get_unit(uint32_t id);
  void for_each_unit(std::function<void(const Unit& unit)> operation);

  // Set the units current path to the path to get them to destination
  void set_path(uint32_t id, const std::vector<sf::Vector3i>& path);
  // Replenish action points to units
  void replenish_actions();

  bool combat(uint32_t attacker_id, uint32_t defender_id);
  // Returns false of the unit damaged is still alive, true if it died.
  bool damage(uint32_t receiver_id, float amount);
  void heal(uint32_t receiver_id, float amount);

  // Changes directoin towards target, doesn't normalize direction.
  void change_direction(uint32_t id, const sf::Vector3i& target);

  void clear();
}
