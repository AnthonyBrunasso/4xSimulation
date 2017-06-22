#pragma once

#include <initializer_list>
#include <vector>

#include "Vector3.hpp"

namespace game_random {
  sf::Vector3i cube_coord(int max_coord);
  void set_seed(unsigned);
  int discrete_distribution(const std::initializer_list<double>& values);
}
