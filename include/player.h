#pragma once

#include <set>
#include <string>
#include <cstdint>
#include <functional>

//
// Player is a generic class that ties units and buildings to some entity.
//

class Player {
public:
  Player(const std::string& name);

  bool OwnsCity(uint32_t id) const;
  bool OwnsUnit(uint32_t id) const;

  std::string m_name;
  // Unique ids of buildings and units that belong to this player
  std::set<uint32_t> m_cities;
  std::set<uint32_t> m_units;
};

namespace player {
  void create(const std::string& name);
  Player* get_player(uint32_t i);

  void add_city(uint32_t player_id, uint32_t city_id);
  void add_unit(uint32_t player_id, uint32_t unit_id);

  void for_each_player(std::function<void(Player& player)> operation);
}
