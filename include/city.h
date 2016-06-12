#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class ConstructionQueueFIFO;

class City {
public:
  explicit City(uint32_t id);

  float GetFoodYield() const;

  void Simulate();
  void BeginTurn() const;

  size_t GetHarvestCount() const;
  bool AddHarvest(sf::Vector3i& );
  bool RemoveHarvest(sf::Vector3i& );

  float GetPopulation() const;
  float FoodForSustain() const;
  float FoodForGrowth() const;
  float GetTurnsForGrowth() const;

  const std::unique_ptr<ConstructionQueueFIFO>& GetConstruction() const;
  const std::unique_ptr<ConstructionQueueFIFO>& GetConstruction();

  uint32_t m_id;
  sf::Vector3i m_location;
  float m_food;
  std::unique_ptr<ConstructionQueueFIFO> m_construction;
  std::vector<sf::Vector3i> m_yield_tiles;
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
  City* nearest_city(sf::Vector3i&);
  City* get_city(uint32_t id);
  void for_each_city(std::function<void(City& )> operation);

  void clear();
}
