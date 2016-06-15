#include "random.h"

#include <iostream>
#include <cstdlib>

sf::Vector3i game_random::cube_coord(int max_coord) { 
  int x = (rand() % (2 * max_coord)) - max_coord;
  int y = 0;
  int t = (max_coord - abs(x));
  if (t) {
    y = (rand() % (2 * t)) - t; 
  }
  int z = -x - y;
  return sf::Vector3i(x, y, z);
}
