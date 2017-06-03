#include "format.h"

#include "city.h"
#include "combat.h"

#include "hex.h"
#include "improvement.h"
#include "player.h"
#include "production.h"
#include "resources.h"
#include "status_effect.h"
#include "step_generated.h"
#include "tile.h"
#include "terrain_yield.h"
#include "unit.h"
#include "util.h"

std::string format::cube_neighbors(const sf::Vector3i& start) {
  hex::CubeNeighbors adj(start);

  std::stringstream ss;
  for (uint32_t i = 0; i < hex::NEIGHBOR_COUNT; ++i) {
    ss << "(" << adj[i].x << "," << adj[i].y << "," << adj[i].z << ") ";
  }

  return (ss.str());
}

std::string format::axial_neighbors(const sf::Vector2i& start) {
  hex::AxialNeighbors adj(start);

  std::stringstream ss;
  for (uint32_t i = 0; i < hex::NEIGHBOR_COUNT; ++i) {
    ss << "(" << adj[i].x << "," << adj[i].y << ") ";
  }

  return (ss.str());
}

std::string format::tile(const Tile& tile) {
  std::stringstream ss;
  ss << "terrain: " << fbs::EnumNameTERRAIN_TYPE(tile.m_terrain_type) << std::endl
     << " base yield: " << terrain_yield::get_base_yield(tile.m_terrain_type) << std::endl
     << " type id: " << static_cast<int32_t>(tile.m_terrain_type) << std::endl
     << " units: " << format::vector(tile.m_unit_ids) << std::endl
     << " city: " << tile.m_city_id << std::endl
     << " path_cost: " << tile.m_path_cost << std::endl
     << " resources: [ ";

  for (auto resource : tile.m_resources) {
    ss << fbs::EnumNameRESOURCE_TYPE(resource.m_type) << ": " << resource.m_quantity << " ";
  }

  ss << "]" << std::endl;

  ss << " improvements: " << format::vector(tile.m_improvement_ids) << std::endl;

  return (ss.str());
}

std::string format::unit(const Unit& unit) {
  std::stringstream ss;

  ss << "unique id: " << unit.m_id << std::endl
     << " unit name: " << fbs::EnumNameUNIT_TYPE(unit.m_type) << std::endl
     << " location: " << format::vector3(unit.m_location) << std::endl
     << " actions: " << unit.m_action_points << std::endl
     << " stats: [" << format::combat_stats(unit.m_combat_stats) << "]" << std::endl
     << " path: " << unit.m_path.size() << std::endl
     << " owner: " << unit.m_owner_id << std::endl
     << " direction: " << fbs::EnumNameDIRECTION_TYPE(util::get_direction(unit.m_direction));

  return (ss.str());
}

std::string format::city(const City& city) {
  std::stringstream ss;

  ss << "City Id: " << city.m_id << " Location: " << format::vector3(city.m_location) << std::endl;
  ss << city.GetPopulation() << " population " << std::endl;
  ss << "    Food: (" << city.FoodForSustain() << " sustain) "
    << "(" << city.m_food << " current) "
     << "(" << city.FoodForGrowth() << " growth) " << std::endl;
  ss << "    Growth: (" << city.FoodForGrowth()-city.m_food << " required) (" << city.GetTurnsForGrowth() << " turns)" << std::endl;
  ss << *city.GetProductionQueue() << std::endl;

  return (ss.str());
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

  return (ascii);
}

std::string format::player(const Player& player) {
  std::stringstream ss;

  ss << "name: " << player.m_name << std::endl;
  ss << " buildings: " << format::set(player.m_cities) << std::endl;
  ss << " units: " << format::set(player.m_units) << std::endl;
  ss << " turn_state: " << fbs::EnumNameTURN_TYPE(player.m_turn_state) << std::endl;
  ss << " gold: " << player.m_gold << std::endl;
  ss << " science: " << player.m_science << std::endl;
  ss << " magic: " << player.m_magic << std::endl;
  ss << " resources: " << format::resources(player::get_resources(player.m_id)) << std::endl;
  ss << " improvements: " << format::set(player.m_improvements) << std::endl;
  ss << " ai: " << fbs::EnumNameAI_TYPE(player.m_ai_type) << std::endl;
  ss << " discovered players: " << format::set(player.m_discovered_players) << std::endl;
  ss << " discovered cities: " << format::set(player.m_discovered_cities) << std::endl;

  return (ss.str());
}

std::string format::combat_stats(const CombatStats& stats) {
  std::stringstream ss;

  ss << "health: " << stats.m_health
     << " attack: " << stats.m_attack
     << " range: " << stats.m_range;

  return (ss.str());
}

std::string format::resources(const ResourceUMap& resources) {
  std::stringstream ss;

  ss << "[ ";
  resources.for_each_resource([&ss](fbs::RESOURCE_TYPE type, const Resource& resource) {
    ss << fbs::EnumNameRESOURCE_TYPE(type) << ": " << resource.m_quantity << " ";
  });
  ss << "]";

  return (ss.str());
}

std::string format::resource(const Resource& resource) {
  std::stringstream ss;
  ss << fbs::EnumNameRESOURCE_TYPE(resource.m_type) << ": " << resource.m_quantity << " ";
  return (ss.str());
}

std::string format::improvement(const Improvement& improvement) {
  std::stringstream ss;

  ss << "Unique id: " << improvement.m_id << std::endl;
  ss << " resource: " << fbs::EnumNameRESOURCE_TYPE(improvement.m_resource.m_type) << std::endl;
  ss << " type: " << fbs::EnumNameIMPROVEMENT_TYPE(improvement.m_type) << std::endl;
  ss << " player id: " << improvement.m_owner_id << std::endl;
  ss << " location: " << format::vector3(improvement.m_location);

  return (ss.str());
}

std::string format::effect(const StatusEffect& effect) {
  std::stringstream ss;

  ss << "Unique id: " << effect.m_id << std::endl;
  ss << " type: " << fbs::EnumNameSTATUS_TYPE(effect.m_type) << std::endl;
  ss << " source location: " << format::vector3(effect.m_location) << std::endl;
  ss << " range: " << effect.m_range << std::endl;
  ss << " total turns: " << effect.m_turns << std::endl;
  ss << " current turn: " << effect.m_current_turn << std::endl;
  ss << " affecting tiles: [ ";
  for (auto t : effect.m_tiles) {
    ss << format::vector3(t) << " ";
  }
  ss << "]" << std::endl;
  ss << " affecting units: " << format::uint_vector(effect.m_units) << std::endl;
  ss << " affecting cities: " << format::uint_vector(effect.m_cities);

  return (ss.str());
}

std::string format::uint_vector(const std::vector<uint32_t>& vec) {
  std::stringstream ss;

  ss << "[ ";

  for (auto ui : vec) {
    ss << ui << " ";
  }

  ss << "]";
  return (ss.str());
}
