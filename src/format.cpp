#include "format.h"

#include "hex.h"

std::string format::cube_neighbors(const sf::Vector3i& start) {
  hex::CubeNeighbors adj(start);

  std::stringstream ss;
  for (uint32_t i = 0; i < hex::NEIGHBOR_COUNT; ++i) {
    ss << "(" << adj[i].x << "," << adj[i].y << "," << adj[i].z << ") ";
  }

  return std::move(ss.str());
}

std::string format::axial_neighbors(const sf::Vector2i& start) {
  hex::AxialNeighbors adj(start);

  std::stringstream ss;
  for (uint32_t i = 0; i < hex::NEIGHBOR_COUNT; ++i) {
    ss << "(" << adj[i].x << "," << adj[i].y << ") ";
  }

  return std::move(ss.str());
}

std::string format::tile(const Tile& tile) {
  std::stringstream ss;

  ss << "unique id: " << tile.m_unique_id
     << " terrain id: " << tile.m_terrain_id
     << " entity ids: [";
  for (uint32_t i = 0; i < tile.m_entity_ids.size(); ++i) {
    ss << tile.m_entity_ids[i] << ",";
  }
  ss << "]";

  return std::move(ss.str());
}