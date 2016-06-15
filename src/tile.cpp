#include "tile.h"

#include "unique_id.h"

Tile::Tile() : 
  m_terrain_type(TERRAIN_TYPE::UNKNOWN)
  , m_unit_ids()
  , m_city_id(unique_id::INVALID_ID)
  // Costs one to walk over the tile by default
  , m_path_cost(1)
  , m_discover_bonus(false)
  , m_resources()
{
}
