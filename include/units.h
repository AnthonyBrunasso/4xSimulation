#pragma once

#include "entity_ids.h"
#include "Vector3.hpp"

#include <cstdint>
#include <functional>

class Unit {
public:
  Unit(uint32_t unique_id, ENTITY_ID entity_id) 
    : m_entity_id(entity_id), 
    m_unique_id(unique_id) {};

  ENTITY_ID m_entity_id;
  uint32_t m_unique_id;
  sf::Vector3i m_location;
};

namespace units {
  uint32_t create(ENTITY_ID entity_id, const sf::Vector3i& location);
  void destroy(uint32_t entity_id);
  Unit* get_unit(uint32_t id);
  void for_each_unit(std::function<void(const Unit& unit)> operation);

  void clear();
}
