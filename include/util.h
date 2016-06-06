#pragma once

#include "Vector3.hpp"

#include <string>
#include <cstdint>

namespace util {
  sf::Vector3i str_to_vector3(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f str_to_vector3f(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f to_vector3f(const sf::Vector3i& vector);
 
  template <class ENUM>
  uint32_t enum_to_uint(ENUM id) {
    return static_cast<uint32_t>(id);
  }

  template <class ENUM>
  ENUM uint_to_enum(uint32_t id) {
    return static_cast<ENUM>(id);
  }
}
