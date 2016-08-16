#pragma once

#include "resources.h"
#include "ai_state.h"

#include <set>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include "game_types.h"

class Unit;
class City;
class Tile;
struct Improvement;

//
// Player is a generic class that ties units and buildings to some ai type.
//

class Player {
public:
  explicit Player(uint32_t id);

  bool OwnsCity(uint32_t id) const;
  bool OwnsUnit(uint32_t id) const;
  bool OwnsImprovement(uint32_t id) const;
  bool DiscoveredPlayer(uint32_t id) const;
  bool DiscoveredCity(uint32_t id) const;
  bool DiscoveredScience(SCIENCE_TYPE st) const;

  uint32_t m_id;
  std::string m_name;
  // Unique ids of buildings and units that belong to this player.
  std::set<uint32_t> m_cities;
  std::set<uint32_t> m_units;
  std::set<uint32_t> m_improvements;
  std::set<uint32_t> m_discovered_players;
  std::set<uint32_t> m_discovered_cities;
  std::set<uint32_t> m_discovered_science;
  std::set<const Tile*> m_discovered_tiles;
  std::vector<uint32_t> m_available_research;
  TURN_TYPE m_turn_state;

  float m_gold;
  float m_science;
  float m_magic;

  // Resources owned by this player.
  SCIENCE_TYPE m_research;
  AI_TYPE m_ai_type;
  // AI state built when needed, could be nullptr for human players.
  std::shared_ptr<AIState> m_ai_state;
};

namespace player {
  uint32_t create_human(const std::string& name);
  uint32_t create_ai();
  Player* get_player(uint32_t i);
  size_t get_count();
  ResourceUMap get_resources(uint32_t player_id);

  void add_city(uint32_t player_id, uint32_t city_id);
  void add_unit(uint32_t player_id, uint32_t unit_id);
  void add_improvement(uint32_t player_id, uint32_t improvement_id);
  void add_discovered_player(uint32_t player_id, uint32_t other_player_id);
  void add_discovered_city(uint32_t player_id, uint32_t city);
  
  bool all_players_turn_ended();

  // Deterministic (sorted) order iteration of the player's objects
  void for_each_player(std::function<void(Player& player)> operation);
  void for_each_player_unit(uint32_t player_id, std::function<void(Unit& unit)> operation);
  void for_each_player_city(uint32_t player_id, std::function<void(City& city)> operation);
  void for_each_player_improvement(uint32_t player_id, std::function<void(Improvement&)>);
  void for_each_player_meet(uint32_t player_id, std::function<void(Player& player)> operation);
  void for_each_city_found(uint32_t player_id, std::function<void(City& city)> operation);
}
