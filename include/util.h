#pragma once

#include "Vector3.hpp"

#include <string>

namespace util {
  sf::Vector3i str_to_vector3(const std::string& x, const std::string& y, const std::string& z);
}