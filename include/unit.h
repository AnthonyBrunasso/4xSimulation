#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "Vector3.hpp"
#include "combat.h"

#include "unique_id.h"

class Player;

namespace fbs {
  enum class UNIT_TYPE : uint32_t;
}

class Unit {
public:
  Unit();
  explicit Unit(uint32_t unique_id);

  fbs::UNIT_TYPE m_type;
  uint32_t m_id;
  sf::Vector3i m_location;
  // Current path the unit is on, size is 0 if the unit is not moving
  std::vector<sf::Vector3i> m_path;
  // Number of actions available for the unit, simulation will dictate this value
  uint32_t m_action_points;
  uint32_t m_owner_id;
  const char* m_name;

  CombatStats m_combat_stats;
  // Vector pointing the way the unit is facing. Directions in this game aren't normalized.
  sf::Vector3i m_direction;
};

struct UnitFatality
{
  explicit UnitFatality(Unit* dead_unit)
    : m_dead(dead_unit)
    , m_opponent(nullptr)
    , m_attacking(nullptr)
  {

  }

  Unit* m_dead;
  Player* m_opponent;
  Unit* m_attacking;
};

namespace unit {
  uint32_t create(fbs::UNIT_TYPE unit_type, const sf::Vector3i& location, uint32_t player_id);
  // Subscribe to creation of a unit
  void sub_create(std::function<void(Unit*)> sub);
  bool destroy(uint32_t dead_id, uint32_t attacking_id, uint32_t opponent_player_id);
  // Subscribe to destruction of a unit
  void sub_destroy(std::function<void(UnitFatality*)> sub);
  Unit* get_unit(uint32_t id);
  void for_each_unit(std::function<void(const Unit& unit)> operation);

  // Set the units current path to the path to get them to destination
  void set_path(uint32_t id, const std::vector<sf::Vector3i>& path);
  // Replenish action points to units
  void replenish_actions();

  bool combat(uint32_t attacker_id, uint32_t defender_id);
  // Returns false of the unit damaged is still alive, true if it died.
  bool damage(uint32_t receiver_id, uint32_t source_player, float amount);
  void heal(uint32_t receiver_id, float amount);

  // Changes direction towards target, doesn't normalize direction.
  void change_direction(uint32_t id, const sf::Vector3i& target);

  size_t size();

  void reset();
}
