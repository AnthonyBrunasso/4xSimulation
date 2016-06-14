#include "random.h"

#include <iostream>
#include <cstdlib>

sf::Vector3i game_random::cube_coord(int max_coord) { 
  int x = (rand() % (2 * max_coord)) - max_coord;
  int t = x == 0 ? 1 : x;
  int y = (rand() % (2 * t)) - t; 
  int z = -x - y;
  return sf::Vector3i(x, y, z);
}
