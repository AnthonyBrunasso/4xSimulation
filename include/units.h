#pragma once

#include "entity_types.h"
#include "Vector3.hpp"

#include <cstdint>
#include <functional>
#include <vector>

class Unit {
public:
  Unit(uint32_t unique_id, ENTITY_TYPE entity_type) 
    : m_entity_type(entity_type)
    , m_unique_id(unique_id)
    , m_location()
    , m_path()
    // Units can do one action per turn by default
    , m_max_actions(1)
    , m_action_points(m_max_actions) {};

  ENTITY_TYPE m_entity_type;
  uint32_t m_unique_id;
  sf::Vector3i m_location;
  // Current path the unit is on, size is 0 if the unit is not moving
  std::vector<sf::Vector3i> m_path;
  // Number of actions available for the unit, simulation will dictate this value
  uint32_t m_max_actions;
  uint32_t m_action_points;
};

namespace units {
  uint32_t create(ENTITY_TYPE entity_type, const sf::Vector3i& location);
  void destroy(uint32_t entity_id);
  Unit* get_unit(uint32_t id);
  void for_each_unit(std::function<void(const Unit& unit)> operation);
  // Move unit forward on it's path by some distance, returns how far it was able to move
  uint32_t move(uint32_t id, uint32_t distance);
  // Replenish action points to units
  void replenish_actions();

  void clear();
}
