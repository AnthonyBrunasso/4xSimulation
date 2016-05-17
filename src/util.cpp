#include "util.h"

sf::Vector3i util::str_to_vector3(const std::string& x, const std::string& y, const std::string& z) {
  const int32_t xi = std::stoi(x);
  const int32_t yi = std::stoi(y);
  const int32_t zi = std::stoi(z);
  return sf::Vector3i(xi, yi, zi);
}

sf::Vector3f util::str_to_vector3f(const std::string& x, const std::string& y, const std::string& z) {
  const float xi = std::stof(x);
  const float yi = std::stof(y);
  const float zi = std::stof(z);
  return sf::Vector3f(xi, yi, zi);
}

sf::Vector3f util::to_vector3f(const sf::Vector3i& vector) {
  const float xi = static_cast<float>(vector.x);
  const float yi = static_cast<float>(vector.y);
  const float zi = static_cast<float>(vector.z);
  return sf::Vector3f(xi, yi, zi);
}