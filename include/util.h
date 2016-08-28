#pragma once

#include "Vector3.hpp"
#include "game_types.h"
#include "simulation.h"
#include "network_types.h"

#include <string>
#include <cstdint>

namespace util {
  sf::Vector3i str_to_vector3(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f str_to_vector3f(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f to_vector3f(const sf::Vector3i& vector);
  DIRECTION_TYPE get_direction(const sf::Vector3i& diff);
  sf::Vector3i get_direction(DIRECTION_TYPE type);
 
  template <class ENUM>
  uint32_t enum_to_uint(ENUM id) {
    return static_cast<uint32_t>(id);
  }

  template <class ENUM>
  ENUM uint_to_enum(uint32_t id) {
    return static_cast<ENUM>(id);
  }

  template<typename T>
  size_t simulate_step(const T& step, char* buffer, size_t buffer_len=256) {
    size_t bytes = serialize(buffer, buffer_len, step);
    simulation::process_step_from_ai(buffer, bytes);
    return bytes;
  }
}
