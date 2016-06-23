#include "magic.h"

#include "tile.h"
#include "world_map.h"
#include "util.h"
#include "units.h"

#include <iostream>
#include <unordered_map>

namespace {
  // A map of damage values done by spells.
  std::unordered_map<uint32_t, float> s_magic_damage; 

  void damage_units(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      std::cout << get_magic_name(type) << " targeted invalid location." << std::endl;
      return;
    }
  
    float dmg = s_magic_damage[util::enum_to_uint(type)];
    for (auto id : tile->m_unit_ids) {
      // Rain ze fire.
      std::cout << "Casting " << get_magic_name(type) << " upon unit " << id << std::endl;
      units::damage(id, dmg);
    }
  }
}

void magic::initialize() {
  // Fireball does 5 whopping damage! Woah!
  s_magic_damage[util::enum_to_uint(MAGIC_TYPE::FIREBALL)] = 5.0f;
}

void magic::cast(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location) {
  switch (type) {
    case MAGIC_TYPE::FIREBALL:
      damage_units(player_id, type, location);
      break;
    case MAGIC_TYPE::UNKNOWN:
      std::cout << "Unkown spell type casted from player_id: " << player_id << std::endl;
      break;
  }
}
