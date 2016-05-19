#include "format.h"

#include "hex.h"
#include "unique_id.h"

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

  ss << "terrain: " << static_cast<uint32_t>(tile.m_terrain_type)
     << " units: " << format::vector(tile.m_unit_ids)
     << " city: " << tile.m_city_id;

  return std::move(ss.str());
}

std::string format::unit(const Unit& unit) {
  std::stringstream ss;

  ss << "unique id: " << unit.m_unique_id
     << " entity id: " << static_cast<uint32_t>(unit.m_entity_type)
     << " location: " << format::vector3(unit.m_location)
     << " actions: " << unit.m_action_points
     << " stats: [" << format::combat_stats(unit.m_combat_stats) << "]";

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

std::string format::ascii_tile(Tile* tile) {
  if (!tile) {
    return "   ";
  }

  std::string ascii = "   ";
  if (tile->m_unit_ids.size()) {
    ascii = " * ";
  }
  if (tile->m_city_id != 0) {
    ascii = " ^ ";
  }

  if (tile->m_unit_ids.size() && tile->m_city_id != 0) {
    ascii = "* ^";
  }

  return std::move(ascii);
}

std::string format::player(const Player& player) {
  std::stringstream ss;

  ss << "name: " << player.m_name 
     << " buildings: " << format::set(player.m_cities)
     << " units: " << format::set(player.m_units);

  return std::move(ss.str());
}

std::string format::combat_stats(const CombatStats& stats) {
  std::stringstream ss;

  ss << "health: " << stats.m_health
     << " attack: " << stats.m_attack
     << " range: " << stats.m_range;

  return std::move(ss.str());
}