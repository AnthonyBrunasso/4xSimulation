#include "city.h"
#include "production.h"

#include "format.h"
#include "hex.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <cmath>

#include "unique_id.h"
#include "tile.h"

namespace city {
  typedef std::unordered_map<uint32_t, City*> CityMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  CityMap s_cities;
  SubMap s_raze_subs;
  SubMap s_create_subs;
  const float FOOD_PER_TURN = 2.f;
}

City::City(uint32_t id)
: m_id(id)
,  m_food(city::food_required_by_population(1)+1)
, m_construction(new ConstructionQueueFIFO())
{ 
}

void City::Simulate() {
  float food = GetFoodYield();
  m_construction->Simulate(this);
  m_food += food;
  std::cout << format::city(*this) << std::endl;
  m_construction->PrintState();
  m_construction->PrintQueue();
}

void City::BeginTurn() const {
  float food = GetFoodYield();
  if (food < 0.0) {
    std::cout << "City is starving, id: " << m_id << std::endl;
  }
  if (m_construction->Count() == 0) {
    std::cout << "City has no constrution orders, id: " << m_id << std::endl;
  }
  float idleCount = static_cast<float>(GetPopulation()-GetHarvestCount());
  if (idleCount) {
    std::cout << "City has " << idleCount << " idle workers, id: " << m_id << std::endl;
  }
}

size_t City::GetHarvestCount() const {
  return m_yield_tiles.size();
}

bool City::AddHarvest(sf::Vector3i &loc) {
  float idleCount = static_cast<float>(GetPopulation()-GetHarvestCount());
  if (idleCount < .001) {
    return false;
  }
  m_yield_tiles.push_back(loc);
  return true;
}

bool City::RemoveHarvest(sf::Vector3i& loc) {
  const auto& findIt = std::find(m_yield_tiles.begin(), m_yield_tiles.end(), loc);
  if (findIt == m_yield_tiles.end()) {
    return false;
  }
  m_yield_tiles.erase(findIt);
  return true;
}

float City::GetFoodYield() const {
  float bonusFood = m_construction->Has(CONSTRUCTION_TYPE::GRANARY)?2.0:0.0;
  return city::FOOD_PER_TURN + bonusFood;
}

float City::GetPopulation() const {
  return city::population_size_from_food(m_food);
}

float City::FoodForSustain() const {
  return city::food_required_by_population(GetPopulation());
}

float City::FoodForGrowth() const {
  return city::food_required_by_population(GetPopulation()+1);
}

float City::GetTurnsForGrowth() const {
  return std::ceil((FoodForGrowth() - m_food) / GetFoodYield());
}

const std::unique_ptr<ConstructionQueueFIFO>& City::GetConstruction() const {
  return m_construction; 
}

const std::unique_ptr<ConstructionQueueFIFO>& City::GetConstruction() {
  return m_construction; 
}

float city::food_required_by_population(float population) {
  return  std::ceil(std::pow(population, 2.75) * 5.f);
}

float city::population_size_from_food(float food) {
  return std::floor(std::pow(food/5.f, (1.f/2.75f)));
}

uint32_t city::create(sf::Vector3i location) {
  uint32_t id = unique_id::generate();

  City* foundedCity = new City(id);
  s_cities[id] = foundedCity;
  foundedCity->m_location = location;
  
  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void city::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void city::raze(uint32_t id) {
  auto findIt = s_cities.find(id);
  if (findIt == s_cities.end()) {
    return;
  }

  // Notify subscribers of razed city
  for (auto sub : s_raze_subs) {
    sub(findIt->second->m_location, id);
  }

  delete findIt->second;
  s_cities.erase(findIt);
}

void city::sub_raze(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_raze_subs.push_back(sub);
}

City* city::nearest_city(sf::Vector3i &loc) {
  uint32_t minDistance = 0xffffffff;
  City* bestFit = nullptr;
  for (auto& cityIt : s_cities) {
     uint32_t distance = hex::cube_distance(loc, cityIt.second->m_location);
     if (distance < minDistance) {
       minDistance = distance;
       bestFit = cityIt.second;
     }
  }

  return bestFit;
}

City* city::get_city(uint32_t id) {
  auto findIt = s_cities.find(id);
  if (findIt == s_cities.end()) {
    return nullptr;
  }

  return findIt->second;
}

void city::for_each_city(std::function<void(City& )> operation) {
  for (auto it = s_cities.begin(); it != s_cities.end(); ++it) {
    operation(*it->second);
  }
}

void city::clear() {
  for (auto it = s_cities.begin(); it != s_cities.end(); ++it) {
    delete it->second;
  }
  s_cities.clear();
}

