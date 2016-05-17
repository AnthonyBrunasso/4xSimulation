#include "city.h"

#include <iostream>
#include <unordered_map>
#include <cmath>

#include "unique_id.h"
#include "world_map.h"
#include "tile.h"

namespace city {
  typedef std::unordered_map<uint32_t, City*> CityMap;
  CityMap s_cities;
  const float FOOD_PER_TURN = 2.f;
}

City::City()
: m_food(city::food_required_by_population(1)+1)
{ 
}

void City::Simulate() {
  m_food += city::FOOD_PER_TURN;
  std::cout << "City is size |" << GetPopulation() << "| "
    << m_food << " food [" << FoodForSustain() << "-" << FoodForGrowth() << "] "
    << "+" << city::FOOD_PER_TURN << " growth in " << GetTurnsForGrowth()
    << std::endl;
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
  return std::ceil((FoodForGrowth() - m_food) / city::FOOD_PER_TURN);
}

float city::food_required_by_population(float population) {
  return  std::ceil(std::pow(population, 2.75) * 5.f);
}

float city::population_size_from_food(float food) {
  return std::floor(std::pow(food/5.f, (1.f/2.75f)));
}

uint32_t city::create(sf::Vector3i location) {
  Tile* tile = world_map::get_tile(location);
  if (!tile) {
    return unique_id::INVALID_ID;
  }

  uint32_t id = unique_id::generate();

  City* foundedCity = new City();
  s_cities[id] = foundedCity;
  foundedCity->m_location = location;
  tile->m_city_id = id;

  return id;
}

void city::raze(uint32_t id)
{
  auto findIt = s_cities.find(id);
  if (findIt == s_cities.end()) {
    return;
  }

  Tile* tile = world_map::get_tile(findIt->second->m_location);
  if (!tile) {
    return;
  }

  tile->m_city_id = unique_id::INVALID_ID;
  delete findIt->second;
  s_cities.erase(findIt);
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

