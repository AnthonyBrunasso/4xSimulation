#pragma once

#include <stdint.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Vector2.hpp"
#include "Vector3.hpp"

class City;
class CombatStats;
class Player;
class StatusEffect;
class Unit;
class Tile;
struct Improvement;
struct Resource;

namespace format {

  template <typename VEC2>
  std::string vector2(const VEC2& vec) {
    std::stringstream ss;
    ss << "v2(" << vec.x << "," << vec.y << ")";
    return std::move(ss.str());
  }

  template <typename VEC3>
  std::string vector3(const VEC3& vec) {
    std::stringstream ss;
    ss << "v3(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return std::move(ss.str());
  }

  template <typename ENTRY>
  std::string vector(const std::vector<ENTRY>& tokens) {
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

  template <typename ENTRY>
  std::string set(const std::set<ENTRY>& tokens) {
    std::stringstream ss;
    ss << "[ ";
    for (auto entry : tokens) {
      ss << entry << " ";
    }
    ss << "]";
    return std::move(ss.str());
  }

  std::string cube_neighbors(const sf::Vector3i& start);
  std::string axial_neighbors(const sf::Vector2i& start);
  std::string tile(const Tile& tile);
  std::string unit(const Unit& unit);
  std::string city(const City& city);
  std::string ascii_tile(Tile* tile);
  std::string player(const Player& player);
  std::string combat_stats(const CombatStats& stats);
  std::string improvement(const Improvement& improvement);
  std::string resource(const Resource& resource);
  std::string effect(const StatusEffect& effect);
  std::string uint_vector(const std::vector<uint32_t>& vec);
}
