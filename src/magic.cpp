#include "magic.h"

#include "tile.h"
#include "world_map.h"
#include "util.h"
#include "units.h"
#include "player.h"

#include <iostream>
#include <unordered_map>

namespace {
  // Compose attributes with different maps. 
  struct Spell {
    Spell() = default;
    Spell(float damage, float cost, MAGIC_TYPE type) :
      m_damage(damage)
      , m_cost(cost)
      , m_type(type) {};

    float m_damage;
    float m_cost;
    MAGIC_TYPE m_type;
  };
  
  std::unordered_map<uint32_t, Spell> s_magic_stats;

  void damage_units(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      std::cout << get_magic_name(type) << " targeted invalid location." << std::endl;
      return;
    }
  
    float dmg = s_magic_stats[util::enum_to_uint(type)].m_damage;
    for (auto id : tile->m_unit_ids) {
      // Rain ze fire.
      std::cout << "Casting " << get_magic_name(type) << " upon unit " << id << std::endl;
      units::damage(id, dmg);
    }
  }
}

void magic::initialize() {
  uint32_t id = util::enum_to_uint(MAGIC_TYPE::FIREBALL);
  // Fireball does 5 whopping damage! Woah!
  s_magic_stats[id] = Spell(5.0f, 2.0f, MAGIC_TYPE::FIREBALL);
}

void magic::cast(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location, bool cheat/*=false*/) {
  float magic_cost = s_magic_stats[util::enum_to_uint(type)].m_cost;
  Player* p = player::get_player(player_id);
  if (!p) {
    std::cout << "Invalid player " << player_id << " attempting to cast a spell." << std::endl;
    return;
  }

  // Cheaters only require the will to cast a spell.
  if (!cheat && p->m_magic < magic_cost) {
    std::cout << "Player " << p->m_name << " doesn't have necessary mana - has " << p->m_magic << "/" << magic_cost << std::endl;
    return;
  }

  switch (type) {
    case MAGIC_TYPE::FIREBALL:
      damage_units(player_id, type, location);
      break;
    case MAGIC_TYPE::UNKNOWN:
      std::cout << "Unkown spell type casted from player_id: " << player_id << std::endl;
      return;
  }

  p->m_magic -= magic_cost;
}
