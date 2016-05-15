#pragma once

#include "tile.h"
#include "Vector2.hpp"
#include "Vector3.hpp"

#include <string>
#include <sstream>

namespace format {

  template <typename VEC2>
  std::string vector2(const VEC2& vec) {
    std::stringstream ss;
    ss << "(" << vec.x << "," << vec.y << ")";
    return ss.str();
  }

  template <typename VEC3>
  std::string vector3(const VEC3& vec) {
    std::stringstream ss;
    ss << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return ss.str();
  }

  std::string cube_neighbors(const sf::Vector3i& start);
  std::string axial_neighbors(const sf::Vector2i& start);
  std::string tile(const Tile& tile);
}