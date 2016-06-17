
#include "terrain_yield.h"
#include "tile.h"
#include "world_map.h"
#include "city.h"
#include "format.h"
#include "game_types.h"

#include <unordered_map>
#include <iostream>

namespace terrain_yield {
  typedef std::unordered_map<sf::Vector3i, uint32_t> YieldMap;
  YieldMap s_terrain_yield;

  bool is_harvested(sf::Vector3i& loc) {
    const auto& itFind = s_terrain_yield.find(loc);
    if (itFind == s_terrain_yield.end()) {
      return false;
    }
    return true;
  }

  bool add_harvest(sf::Vector3i& loc, City* owner) {
    if (!owner->AddHarvest(loc)) {
      std::cout << "City refused harvest (no idle workers, too far away, etc" << std::endl;
      return false;
    }
    s_terrain_yield[loc] = owner->m_id;
    return true;
  }

  void remove_harvest(sf::Vector3i& loc) {
    const auto& itFind = s_terrain_yield.find(loc);
    if (itFind == s_terrain_yield.end()) {
      return;
    }
    uint32_t cityId = itFind->second;
    s_terrain_yield.erase(itFind);

    City* city = city::get_city(cityId);
    if (!city) {
      return;
    }
    city->RemoveHarvest(loc);
    std::cout << "Toggle: City (" << city->m_id << ") is no longer harvesting from " << format::vector3(loc) << std::endl;
  }
}

TerrainYield::TerrainYield()
: m_type(TERRAIN_TYPE::UNKNOWN)
, m_food(0.f)
, m_production(0.f)
, m_science(0.f)
, m_gold(0.f)
, m_experience(0.f)
{
}

void TerrainYield::operator+=(const TerrainYield& rhs) {
  m_food += rhs.m_food;
  m_production += rhs.m_production;
  m_science += rhs.m_science;
  m_gold += rhs.m_gold;
  m_experience += rhs.m_experience;
}

const TerrainYield operator+(const TerrainYield& lhs, const TerrainYield& rhs) {
  TerrainYield base;
  base.m_food = lhs.m_food + rhs.m_food;
  base.m_production = lhs.m_production + rhs.m_production;
  base.m_science = lhs.m_science + rhs.m_science;
  base.m_gold = lhs.m_gold + rhs.m_gold;
  base.m_experience = lhs.m_experience + rhs.m_experience;
  return std::move(base);
}

std::ostream& operator<<(std::ostream& out,  const TerrainYield& ty) {
  out << "TerrainYield "
      << ((ty.m_type != TERRAIN_TYPE::UNKNOWN)?get_terrain_name(ty.m_type):"")
      << "(" << ty.m_food << " Food) "
      << "(" << ty.m_production << " Prod) "
      << "(" << ty.m_science << " Sci) "
      << "(" << ty.m_gold << " Gold) "
      << "(" << ty.m_experience << " Xp)";
  return out;
}

namespace terrain_yield {
  void DesertYield(TerrainYield& t) {
    t.m_production += 1;
  }
  
  void GrasslandYield(TerrainYield& t) {
    t.m_food += 2;
  }
  
  void PlainsYield(TerrainYield& t) {
    t.m_food += 1;
    t.m_production += 1;
  }
  
  void MountainYield(TerrainYield& t) {
    t.m_production += 1;
  }
  
  void WaterYield(TerrainYield& t) {
    t.m_food += 1;
    t.m_gold += 1;
  }

  void DesertSpecialization(TerrainYield& t) {
    t.m_experience += 2;
  }

  void GrasslandSpecialization(TerrainYield& t) {
    t.m_food += 1;
    t.m_experience += 1;
  }

  void PlainsSpecialization(TerrainYield& t) {
    t.m_science += 1;
    t.m_experience += 1;
  }

  void MountainSpecialization(TerrainYield& t) {
    t.m_production += 1;
    t.m_experience += 1;
  }

  void WaterSpecialization(TerrainYield& t) {
    t.m_gold += 1;
    t.m_experience += 1;
  }
  
  typedef std::unordered_map<int32_t, std::function<void (TerrainYield&)> > YieldFunctions;
  static YieldFunctions s_defaultYieldFn{
    {static_cast<int32_t>(TERRAIN_TYPE::DESERT), &DesertYield},
    {static_cast<int32_t>(TERRAIN_TYPE::GRASSLAND), &GrasslandYield},
    {static_cast<int32_t>(TERRAIN_TYPE::MOUNTAIN), &MountainYield},
    {static_cast<int32_t>(TERRAIN_TYPE::PLAINS), &PlainsYield},
    {static_cast<int32_t>(TERRAIN_TYPE::WATER), &WaterYield},
  };
  static YieldFunctions s_specializeYieldFn{
    {static_cast<int32_t>(TERRAIN_TYPE::DESERT), &DesertSpecialization},
    {static_cast<int32_t>(TERRAIN_TYPE::GRASSLAND), &GrasslandSpecialization},
    {static_cast<int32_t>(TERRAIN_TYPE::MOUNTAIN), &MountainSpecialization},
    {static_cast<int32_t>(TERRAIN_TYPE::PLAINS), &PlainsSpecialization},
    {static_cast<int32_t>(TERRAIN_TYPE::WATER), &WaterSpecialization},
  };

  TerrainYield get_yield(sf::Vector3i loc, TERRAIN_TYPE spec) {
    TerrainYield base = TerrainYield();
    Tile* tile = world_map::get_tile(loc);
    if (!tile) {
      return base;
    }
    // Calculate the default yields
    base = terrain_yield::get_base_yield(tile->m_terrain_type);
    
    if (tile->m_terrain_type == spec) {
      base += terrain_yield::get_specialization_yield(spec);
    }
    return base;
  }
}

TerrainYield terrain_yield::get_base_yield(TERRAIN_TYPE type) {
  TerrainYield base;
  base.m_type = type;
  const auto& findIt = s_defaultYieldFn.find(static_cast<int32_t>(type));
  if (findIt == s_defaultYieldFn.end()) {
    return base;
  }

  findIt->second(base);
  return base;
}

TerrainYield terrain_yield::get_specialization_yield(TERRAIN_TYPE type) {
  TerrainYield base;
  const auto& findIt = s_specializeYieldFn.find(static_cast<int32_t>(type));
  if (findIt == s_specializeYieldFn.end()) {
    return base;
  }
  
  findIt->second(base);
  return base;
}

