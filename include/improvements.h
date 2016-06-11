#pragma once

#include "game_types.h"

#include <Vector3.hpp>
#include <cstdint>
#include <functional>

struct Improvement {
  Improvement(uint32_t unique_id, IMPROVEMENT_TYPE type);

  uint32_t m_unique_id;
  IMPROVEMENT_TYPE m_type;
  // This is the player that owns this resource
  uint32_t m_owner_id;
  sf::Vector3i m_location;
};

namespace improvement {
  // Improvements will often have requirements. For example, improving a resource
  // requires a resource exist on that tile. Building a road requires that the player
  // the necessary research to build it.
  void add_requirement(IMPROVEMENT_TYPE type, std::function<bool(const sf::Vector3i&)> requirement);
  uint32_t create(IMPROVEMENT_TYPE type, const sf::Vector3i& location, uint32_t owner);
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  void destroy(uint32_t id);
  void sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  void for_each_improvement(
      std::function<void(const Improvement& improvement)> operation);
  Improvement* get_improvement(uint32_t id);
}
