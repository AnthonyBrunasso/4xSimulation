#pragma once

#include "Vector3.hpp"
#include "game_types.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class ConstructionQueueFIFO;
struct TerrainYield;

class City {
public:
  explicit City(uint32_t id);

  bool CanSpecialize() const;
  bool SetSpecialization(TERRAIN_TYPE type);

  void Simulate(TerrainYield&);
  void BeginTurn() const;

  TerrainYield DumpYields(bool log=false) const;
  void MutateYield(TerrainYield&) const;

  size_t GetHarvestCount() const;
  bool AddHarvest(sf::Vector3i& );
  bool RemoveHarvest(sf::Vector3i& );

  float GetPopulation() const;
  float FoodForSustain() const;
  float FoodForGrowth() const;
  float GetTurnsForGrowth() const;

  bool IsConstructing() const;

  const std::unique_ptr<ConstructionQueueFIFO>& GetConstruction() const;
  const std::unique_ptr<ConstructionQueueFIFO>& GetConstruction();

  uint32_t m_id;
  sf::Vector3i m_location;
  float m_food;
  float m_experience;
  TERRAIN_TYPE m_specialization;
  std::unique_ptr<ConstructionQueueFIFO> m_construction;
  std::vector<sf::Vector3i> m_yield_tiles;
  uint32_t m_owner_id;
};

namespace city {
  // Utils
  float food_required_by_population(float population);
  float population_size_from_food(float food);

  // Access
  void add_requirement(BUILDING_TYPE type, 
    std::function<bool(const sf::Vector3i&, uint32_t)> requirement);
  uint32_t create(BUILDING_TYPE type, const sf::Vector3i& location, uint32_t player_id);
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  void raze(uint32_t id);
  void sub_raze(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  City* nearest_city(sf::Vector3i&);
  City* get_city(uint32_t id);
  void for_each_city(std::function<void(City& )> operation);

  void clear();
}
