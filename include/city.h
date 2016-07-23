#pragma once

#include "Vector3.hpp"
#include "game_types.h"
#include "notification.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class ConstructionQueueFIFO;
struct TerrainYield;

class City {
public:
  explicit City(uint32_t id);
  City(const City&) = default;

  bool CanSpecialize() const;
  bool SetSpecialization(TERRAIN_TYPE type);

  void Siege(float damage);
  bool Capture();
  
  void Simulate(TerrainYield&);

  TerrainYield DumpYields(bool log=false) const;
  void MutateYield(TerrainYield&) const;

  size_t GetHarvestCount() const;
  bool AddHarvest(sf::Vector3i& );
  bool RemoveHarvest(sf::Vector3i& );
  void RemoveAllHarvest();

  float IdleWorkers() const;
  float GetPopulation() const;
  float FoodForSustain() const;
  float FoodForGrowth() const;
  float GetTurnsForGrowth() const;

  bool IsConstructing() const;

  ConstructionQueueFIFO* GetConstruction() const;
  void Purchase(CONSTRUCTION_TYPE t);

  uint32_t m_id;
  sf::Vector3i m_location;
  float m_food;
  float m_experience;
  float m_damage;
  bool m_razing;
  bool m_defenses_used;
  TERRAIN_TYPE m_specialization;
  uint32_t m_production_id;
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
  void sub_raze_init(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  void sub_raze_complete(std::function<void(const sf::Vector3i&, uint32_t)> sub);
  City* nearest_city(sf::Vector3i&);
  City* get_city(uint32_t id);
  void do_notifications(uint32_t city_id, NotificationVector& events);
  void for_each_city(std::function<void(City& )> operation);

  void clear();
}
