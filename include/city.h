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
  void create(uint32_t id, sf::Vector3i);
  void raze(uint32_t id);
  City* get_city(uint32_t id);
  void for_each_city(std::function<void(City& )> operation);

  void clear();
}
