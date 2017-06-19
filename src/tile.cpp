#include "tile.h"

#include <algorithm>

#include "unique_id.h"
#include "step_generated.h"

Tile::Tile() : 
  m_terrain_type(fbs::TERRAIN_TYPE::UNKNOWN)
  , m_unit_ids()
  , m_city_id(unique_id::INVALID_ID)
  // Costs one to walk over the tile by default
  , m_path_cost(1)
  , m_discover_bonus(false)
  , m_improvement_ids()
  , m_location()
  , m_status_ids()
{
  m_resource.m_type = fbs::RESOURCE_TYPE::UNKNOWN;
}

Tile::Tile(sf::Vector3i loc) : 
  m_terrain_type(fbs::TERRAIN_TYPE::UNKNOWN)
  , m_unit_ids()
  , m_city_id(unique_id::INVALID_ID)
  // Costs one to walk over the tile by default
  , m_path_cost(1)
  , m_discover_bonus(false)
  , m_improvement_ids()
  , m_location(loc)
  , m_status_ids()
{
  m_resource.m_type = fbs::RESOURCE_TYPE::UNKNOWN;
}

