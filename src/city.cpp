#include "city.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "format.h"
#include "hex.h"
#include "production.h"
#include "search.h"
#include "step_generated.h"
#include "terrain_yield.h"
#include "unique_id.h"
#include "unit.h"
#include "util.h"
#include "world_map.h"

namespace city {
  typedef std::unordered_map<uint32_t, City*> CityMap;
  CityMap s_cities;

  typedef std::function<bool(const sf::Vector3i&, uint32_t)> SubscriberFunc;
  constexpr size_t SUBSCRIBER_LIMIT = 10;
  SubscriberFunc s_raze_init_subs[SUBSCRIBER_LIMIT];
  SubscriberFunc s_raze_complete_subs[SUBSCRIBER_LIMIT];
  SubscriberFunc s_create_subs[SUBSCRIBER_LIMIT];

  void notify_raze_init(City& c);
  void notify_raze_complete(City& c);
}

City::City(uint32_t id)
: m_id(id)
, m_food(city::food_required_by_population(1)-1)
, m_experience(0.f)
, m_damage(0.f)
, m_razing(false)
, m_defenses_used(false)
, m_specialization(fbs::TERRAIN_TYPE::UNKNOWN)
, m_production_id(0)
{ 
}

bool City::CanSpecialize() const {
  return (m_experience > 3.0f);
}

bool City::SetSpecialization(fbs::TERRAIN_TYPE type) {
  if (!CanSpecialize()) {
    return false;
  }
  // May only be set once
  if (m_specialization != fbs::TERRAIN_TYPE::UNKNOWN) {
    return false;
  }
  m_specialization = type;
  return true;
}

void City::Siege(float damage) {
  m_damage += damage;
  std::cout << "This city (" << m_id << ") has taken " << m_damage << " damage." << std::endl;
  if (m_damage > MaxHealth()) {
    std::cout << "This city (" << m_id << ") has begun to burn to the ground." << std::endl;
    m_razing = true;
    city::notify_raze_init(*this);
  }
}

bool City::Capture() {
  // City may be captured only when it has begun razing
  return m_razing;
}

void City::Simulate(TerrainYield& t) {
  if (m_razing) {
    // Construction is not processed
    if (GetPopulation() > 0.f) {
      // City shrinks in size
      m_food = city::food_required_by_population(GetPopulation()-1);
      std::cout << "City shrinks, food: " << m_food << std::endl;
    }
    
    // City may be deleted above
    return;
  }
  
  // Auto-repair
  m_damage = std::max(0.f, m_damage-3.f);

  // Normal city functions
  production_queue::simulate(GetProductionQueue(), t);
  m_food += t.m_food;
  m_experience += t.m_experience;
  std::cout << format::city(*this);
}

TerrainYield City::DumpYields(bool log) const {
  TerrainYield sum;
  for (const auto& it : m_yield_tiles) {
    TerrainYield yieldVal = terrain_yield::get_yield(it, m_specialization); 
    if (log) {
      std::cout << "    " << yieldVal << std::endl;
    }
    sum += yieldVal;
  }
  if (log) {
    std::cout << "  Sum yield: " << sum << std::endl;
  }
  MutateYield(sum);
  if (log) {
    std::cout << "Total yield (city " << m_id << " ): " << sum << std::endl;
  }
  return sum;
}

void City::MutateYield(TerrainYield& yields) const {
  float bonusFood = 2.f;
  yields.m_food += bonusFood;
  yields.m_experience += 1.f;
  yields += production_queue::yield(GetProductionQueue());
}

size_t City::GetHarvestCount() const {
  return m_yield_tiles.size();
}

bool City::AddHarvest(sf::Vector3i &loc) {
  float idleCount = IdleWorkers();
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

void City::RemoveAllHarvest() {
  for (size_t i = 0; i < m_yield_tiles.size(); ++i) {
    terrain_yield::remove_harvest(m_yield_tiles[i]);
  }
}

float City::MaxHealth() const {
  return GetPopulation()*10.f;
}

float City::IdleWorkers() const {
  return static_cast<float>(GetPopulation()-GetHarvestCount());
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
  TerrainYield t = DumpYields();
  return std::ceil((FoodForGrowth() - m_food) / t.m_food);
}

bool City::IsConstructing() const {
  return !GetProductionQueue()->m_queue.empty();
}

ConstructionQueueFIFO* City::GetProductionQueue() const {
  return production::get_production(m_production_id);
}

float city::food_required_by_population(float population) {
  return std::ceil(std::pow(population, 2.75f) * 5.f);
}

float city::population_size_from_food(float food) {
  return std::floor(std::pow(food/5.f, (1.f/2.75f)));
}

uint32_t city::create(const sf::Vector3i& location, uint32_t player_id) {
  uint32_t id = unique_id::generate();

  City* foundedCity = new City(id);
  foundedCity->m_production_id = production::create(id);
  s_cities[id] = foundedCity;
  foundedCity->m_location = location;
  foundedCity->m_owner_id = player_id;
  
  for (auto sub : s_create_subs) {
    if (sub) {
      sub(location, id); 
    }
  }

  return id;
}

void city::sub_create(std::function<bool(const sf::Vector3i&, uint32_t)> sub) {
  for (auto &s : s_create_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
}

void city::raze(uint32_t id) {
  std::cout << "raze called " << id << std::endl;
  auto findIt = s_cities.find(id);
  if (findIt == s_cities.end()) {
    return;
  }

  findIt->second->RemoveAllHarvest();

  // Notify subscribers of razed city
  city::notify_raze_complete(*findIt->second);

  delete findIt->second;
  s_cities.erase(findIt);
}

void city::notify_raze_init(City& c) {
  for (auto sub: s_raze_init_subs) {
    if (sub) {
      sub(c.m_location, c.m_id);
    }
  }
}

void city::sub_raze_init(std::function<bool(const sf::Vector3i&, uint32_t)> sub) {
  for (auto &s : s_raze_init_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
}

void city::notify_raze_complete(City& c) {
  for (auto sub: s_raze_complete_subs) {
    if (sub) {
      sub(c.m_location, c.m_id);
    }
  }
}

void city::sub_raze_complete(std::function<bool(const sf::Vector3i&, uint32_t)> sub) {
  for (auto &s : s_raze_complete_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
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

void city::do_notifications(uint32_t id, NotificationVector& events) {
  City* c = get_city(id);
  TerrainYield t = c->DumpYields();
  if (t.m_food < 0.0) {
    Notification n;
    n.m_event_type = fbs::NOTIFICATION_TYPE::CITY_STARVING;
    n.m_id = id;
    events.push_back(n);
  }
  if (!c->IsConstructing()) {
    Notification n;
    n.m_event_type = fbs::NOTIFICATION_TYPE::CITY_PRODUCTION;
    n.m_id = id;
    events.push_back(n);
  }
  float idleCount = c->IdleWorkers();
  if (idleCount) {
    Notification n;
    n.m_event_type = fbs::NOTIFICATION_TYPE::CITY_HARVEST;
    n.m_id = id;
    events.push_back(n);
  }
  if (c->m_specialization == fbs::TERRAIN_TYPE::UNKNOWN && c->CanSpecialize()) {
    Notification n;
    n.m_event_type = fbs::NOTIFICATION_TYPE::CITY_SPECIALIZE;
    n.m_id = id;
    events.push_back(n);
  }
  search::bfs_units(c->m_location,
    2,
    world_map::get_map(),
    [&events, c](const Unit& u) {
      if (u.m_owner_id == c->m_owner_id) {
        return false;
      }
      
      Notification n;
      n.m_event_type = fbs::NOTIFICATION_TYPE::CITY_DEFENSE;
      n.m_id = c->m_id;
      n.m_other_id = u.m_id;
      events.push_back(n);
      return true;
    }
  );

}

void city::for_each_city(std::function<void(City& )> operation) {
  for (auto it = s_cities.begin(); it != s_cities.end(); ++it) {
    operation(*it->second);
  }
}

void city::reset() {
  for (auto it = s_cities.begin(); it != s_cities.end(); ++it) {
    delete it->second;
  }
  s_cities.clear();

  for (auto &s : s_raze_init_subs) {
    s = SubscriberFunc();
  }
  for (auto &s : s_raze_complete_subs) {
    s = SubscriberFunc();
  }
  for (auto &s : s_create_subs) {
    s = SubscriberFunc();
  }
}

