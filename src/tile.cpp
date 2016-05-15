#include "tile.h"

namespace {
  static uint32_t s_unique_tile_ids = 0;
}

Tile::Tile() : m_unique_id(s_unique_tile_ids++), m_terrain_id(0) {}