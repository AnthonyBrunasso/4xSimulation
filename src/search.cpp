#include "search.h"

#include <iostream>
#include <algorithm>

void search::range(const sf::Vector3i& start, int32_t distance, std::vector<sf::Vector3i>& coords) {
  for (int32_t dx = -distance; dx <= distance; ++dx) {
    int32_t s = std::max(-distance, -dx - distance);
    for (int32_t dy = s; dy <= std::min(distance, -dx + distance); ++dy) {
      int32_t dz = -dx - dy;
      coords.push_back(start + sf::Vector3i(dx, dy, dz));
    }
  }
}