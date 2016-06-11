#include "format.h"
#include "city.h"
#include "production.h"

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
  ss << "terrain: " << static_cast<uint32_t>(tile.m_terrain_type) << std::endl
     << " units: " << format::vector(tile.m_unit_ids) << std::endl
     << " city: " << tile.m_city_id << std::endl
     << " path_cost: " << tile.m_path_cost << std::endl
     << " resources: [ ";

  for (auto resource : tile.m_resources) {
    ss << get_resource_name(resource.m_type) << ": " << resource.m_quantity << " ";
  }

  ss << "]" << std::endl;

  ss << " improvements: " << format::vector(tile.m_improvement_ids) << std::endl;

  return std::move(ss.str());
}

std::string format::unit(const Unit& unit) {
  std::stringstream ss;

  ss << "unique id: " << unit.m_unique_id << std::endl
     << " unit name: " << get_unit_name(unit.m_unit_type) << std::endl
     << " location: " << format::vector3(unit.m_location) << std::endl
     << " actions: " << unit.m_action_points << std::endl
     << " stats: [" << format::combat_stats(unit.m_combat_stats) << "]" << std::endl
     << " path: " << unit.m_path.size();

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
  if (!tile->m_resources.empty()) {
    ascii = " ! ";
  }

  if (tile->m_unit_ids.size() && tile->m_city_id != 0) {
    ascii = "* ^";
  }

  return std::move(ascii);
}

std::string format::player(const Player& player) {
  std::stringstream ss;

  ss << "name: " << player.m_name << std::endl;
  ss << " buildings: " << format::set(player.m_cities) << std::endl;
  ss << " units: " << format::set(player.m_units) << std::endl;
  ss << " turn_state: " << static_cast<size_t>(player.m_turn_state) << std::endl;
  ss << " resources: " << format::resources(player.m_resources) << std::endl;
  ss << " improvements: " << format::set(player.m_improvements) << std::endl;

  return std::move(ss.str());
}

std::string format::combat_stats(const CombatStats& stats) {
  std::stringstream ss;

  ss << "health: " << stats.m_health
     << " attack: " << stats.m_attack
     << " range: " << stats.m_range;

  return std::move(ss.str());
}

std::string format::resources(const Resources& resources) {
  std::stringstream ss;

  ss << "[ ";
  resources.for_each_resource([&ss](RESOURCE_TYPE type, const Resource& resource) {
    ss << get_resource_name(type) << ": " << resource.m_quantity << " ";
  });
  ss << "]";

  return std::move(ss.str());
}

std::string format::improvement(const Improvement& improvement) {
  std::stringstream ss;

  ss << "Unique id: " << improvement.m_unique_id << std::endl;
  ss << " type: " << get_improvement_name(improvement.m_type) << std::endl;
  ss << " player id: " << improvement.m_owner_id << std::endl;
  ss << " location: " << format::vector3(improvement.m_location);

  return std::move(ss.str());
}
