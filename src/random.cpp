#include "random.h"

#include <stdlib.h>
#include <random>

namespace game_random {
  std::mt19937 s_mt;
}

sf::Vector3i game_random::cube_coord(int max_coord) { 
  int x = (s_mt() % (2 * max_coord)) - max_coord;
  int y = 0;
  int t = (max_coord - abs(x));
  if (t) {
    y = (s_mt() % (2 * t)) - t; 
  }
  int z = -x - y;
  return sf::Vector3i(x, y, z);
}

void game_random::set_seed(unsigned seed) {
  s_mt.seed(seed);
}
