#include "random.h"


#include <cstdlib>

sf::Vector3i game_random::cube_coord(int max_coord) { 
  int x = (rand() % (2 * max_coord)) - max_coord;
  int y = (rand() % (2 * x)) - x; 
  int z = -x - y;
  return sf::Vector3i(x, y, z);
}
