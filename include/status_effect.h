#pragma once

#include "game_types.h"
#include "Vector3.hpp"

#include <cstdint>
#include <functional>

class StatusEffect {
public:
  StatusEffect(uint32_t id, STATUS_TYPE type, const sf::Vector3i& location) :
    m_id(id)
    , m_type(type)
    , m_location(location)
    , m_turns(0) {};

  uint32_t m_id;
  STATUS_TYPE m_type;
  sf::Vector3i m_location;
  uint32_t m_turns;
};

namespace status_effect {
  uint32_t create(STATUS_TYPE type, const sf::Vector3i& location);  
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  void destroy(uint32_t id);
  void sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  StatusEffect* get_effect(uint32_t id);
  void for_each_effect(std::function<void(const StatusEffect& effect)> operation);
}
