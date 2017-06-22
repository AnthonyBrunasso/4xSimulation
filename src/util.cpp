#include "util.h"

#include <cstdint>

#include "enum_generated.h"

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

fbs::DIRECTION_TYPE util::get_direction(const sf::Vector3i& diff) {
  if (diff == sf::Vector3i(1, -1, 0)) {
    return fbs::DIRECTION_TYPE::NORTH_EAST;
  }
  else if (diff == sf::Vector3i(1, 0, -1)) {
    return fbs::DIRECTION_TYPE::EAST;
  }
  else if (diff == sf::Vector3i(0, 1, -1)) {
    return fbs::DIRECTION_TYPE::SOUTH_EAST;
  }
  else if (diff == sf::Vector3i(-1, 1, 0)) {
    return fbs::DIRECTION_TYPE::SOUTH_WEST;
  }
  else if (diff == sf::Vector3i(-1, 0, 1)) {
    return fbs::DIRECTION_TYPE::WEST;
  }
  else if (diff == sf::Vector3i(0, -1, 1)) {
    return fbs::DIRECTION_TYPE::NORTH_WEST;
  }
  return fbs::DIRECTION_TYPE::UNKNOWN;
}

sf::Vector3i util::get_direction(fbs::DIRECTION_TYPE type) {
  switch (type) {
    case fbs::DIRECTION_TYPE::NORTH_EAST:
      return sf::Vector3i(1, -1, 0);
    case fbs::DIRECTION_TYPE::EAST:
      return sf::Vector3i(1, 0, -1);
    case fbs::DIRECTION_TYPE::SOUTH_EAST:
      return sf::Vector3i(0, 1, -1);
    case fbs::DIRECTION_TYPE::SOUTH_WEST:
      return sf::Vector3i(-1, 1, 0);
    case fbs::DIRECTION_TYPE::WEST:
      return sf::Vector3i(-1, 0, 1);
    case fbs::DIRECTION_TYPE::NORTH_WEST:
      return sf::Vector3i(0, -1, 1);
    case fbs::DIRECTION_TYPE::UNKNOWN:
      return sf::Vector3i(0, 0, 0);
  }
  return sf::Vector3i(0, 0, 0);
}

size_t util::enum_from_names(const std::string& searchName, const char** names) {                          
	size_t index = 0;                                                                                         
	while (*names) {                                                                                          
		if (searchName == *names) {                                                                             
			return (index);                                                                                       
		}                                                                                                       
		++names;                                                                                                
		++index;                                                                                                
	}                                                                                                         
																																																						
	return (0);                                                                                               
}

