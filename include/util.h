#pragma once

#include "Vector3.hpp"
#include "game_types.h"

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

  template <class ENUM>
  ENUM enum_from_names(const std::string& searchName, const char** names) {
    size_t index = 0;
    while (*names) {
      if (searchName == *names) {
        return static_cast<ENUM>(index);
      }
      ++names;
      ++index;
    }

    return static_cast<ENUM>(0);
  }
}
