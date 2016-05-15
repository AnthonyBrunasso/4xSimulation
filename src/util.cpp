#include "util.h"

sf::Vector3i util::str_to_vector3(const std::string& x, const std::string& y, const std::string& z) {
  const int32_t xi = std::stoi(x);
  const int32_t yi = std::stoi(y);
  const int32_t zi = std::stoi(z);
  return sf::Vector3i(xi, yi, zi);
}