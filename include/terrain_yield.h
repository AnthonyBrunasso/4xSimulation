#include "Vector3.hpp"
#include <iostream>

class City;
namespace fbs {
  enum class TERRAIN_TYPE : uint32_t;
  enum class RESOURCE_TYPE : uint32_t;
};

struct TerrainYield
{
  TerrainYield();

  void operator+=(const TerrainYield& rhs);

  fbs::TERRAIN_TYPE m_type;
  float m_food;
  float m_production;
  float m_science;
  float m_gold;
  float m_experience;
  float m_magic;
};

const TerrainYield operator+(const TerrainYield& lhs, const TerrainYield& rhs);
std::ostream& operator<<(std::ostream&, const TerrainYield&);

namespace terrain_yield {
  bool is_harvested(sf::Vector3i& loc);
  bool add_harvest(sf::Vector3i& loc, City*);
  void remove_harvest(sf::Vector3i& loc);

  TerrainYield get_yield(const sf::Vector3i& loc, fbs::TERRAIN_TYPE specialization);
  TerrainYield get_base_yield(fbs::TERRAIN_TYPE type);
  TerrainYield get_specialization_yield(fbs::TERRAIN_TYPE type);
  TerrainYield get_resource_yield(fbs::RESOURCE_TYPE);

  void reset();
}
