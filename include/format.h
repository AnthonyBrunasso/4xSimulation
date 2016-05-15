#pragma once

#include "tile.h"
#include "Vector2.hpp"
#include "Vector3.hpp"

#include <string>
#include <sstream>
#include <vector>

namespace format {

  template <typename VEC2>
  std::string vector2(const VEC2& vec) {
    std::stringstream ss;
    ss << "(" << vec.x << "," << vec.y << ")";
    return std::move(ss.str());
  }

  template <typename VEC3>
  std::string vector3(const VEC3& vec) {
    std::stringstream ss;
    ss << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return std::move(ss.str());
  }

  template <typename ENTRY>
  std::string tokens(const std::vector<ENTRY>& tokens) {
    std::stringstream ss;
    ss << "[";
    for (uint32_t i = 0; i < tokens.size(); ++i) {
      ss << tokens[i];
      if (i != tokens.size() - 1) {
        ss << ",";
      }
    }
    ss << "]";
    return std::move(ss.str());
  }

  std::string cube_neighbors(const sf::Vector3i& start);
  std::string axial_neighbors(const sf::Vector2i& start);
  std::string tile(const Tile& tile);
}