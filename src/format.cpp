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
     << " entity ids: " << format::vector(tile.m_occupied_ids);

  return std::move(ss.str());
}

std::string format::unit(const Unit& unit) {
  std::stringstream ss;

  ss << "unique id: " << unit.m_unique_id
     << " entity id: " << static_cast<uint32_t>(unit.m_entity_id)
     << " location: " << format::vector3(unit.m_location);

  return std::move(ss.str());
}

std::string format::city(const City& city) {
  std::stringstream ss;

  ss << "food: " << city.m_food
     << " population: " << city.GetPopulation()
     << " sustain: " << city.FoodForSustain()
     << " growth: " << city.FoodForGrowth()
     << " turns for growth: " << city.GetTurnsForGrowth()
     << " location: " << format::vector3(city.m_location);

  return std::move(ss.str());
}