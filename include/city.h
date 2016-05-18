#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <functional>

class City {
public:
  City();

  void Simulate();
  float GetPopulation() const;
  float FoodForSustain() const;
  float FoodForGrowth() const;
  float GetTurnsForGrowth() const;


  sf::Vector3i m_location;
  float m_food;
};

namespace city {
  // Utils
  float food_required_by_population(float population);
  float population_size_from_food(float food);

  // Access
  uint32_t create(sf::Vector3i);
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  void raze(uint32_t id);
  void sub_raze(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  City* get_city(uint32_t id);
  void for_each_city(std::function<void(City& )> operation);

  void clear();
}
