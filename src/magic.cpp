#include "magic.h"

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "city.h"
#include "player.h"
#include "search.h"
#include "step_generated.h"
#include "tile.h"
#include "unit.h"
#include "util.h"
#include "world_map.h"

namespace magic {
  // Compose attributes with different maps. 
  struct Spell {
    Spell() = default;
    Spell(float damage, float cost, fbs::MAGIC_TYPE type) :
      m_damage(damage)
      , m_cost(cost)
      , m_type(type) {};

    float m_damage;
    float m_cost;
    fbs::MAGIC_TYPE m_type;
  };
  
  constexpr size_t MAGIC_LIMIT = (size_t)fbs::MAGIC_TYPE::MAX+1;
  Spell s_magic_stats[MAGIC_LIMIT];

  bool damage_units(uint32_t casting_player, fbs::MAGIC_TYPE type, const Tile& tile) {
    uint32_t idx = any_enum(type);
    float dmg = s_magic_stats[idx].m_damage;
    for (auto id : tile.m_unit_ids) {
      // Rain ze fire.
      std::cout << "Casting " << fbs::EnumNameMAGIC_TYPE(type) << " upon unit " << id << std::endl;
      // If a unit died, jump out since tile->m_unit_ids will mutate.
      if (unit::damage(id, casting_player, dmg)) break;
    }

    return false;
  }

  bool missile_requirements(const sf::Vector3i& location, uint32_t player_id);
  bool fireball_requirements(const sf::Vector3i& location, uint32_t player_id);
  void aoe_damage(uint32_t casting_player, fbs::MAGIC_TYPE type, const Tile& origin) {
    search::bfs(origin.m_location, 1, world_map::get_map(), std::bind(damage_units, casting_player, type, std::placeholders::_1));
  }
}

void magic::initialize() {
  uint32_t fb_id = any_enum(fbs::MAGIC_TYPE::FIREBALL);
  // Fireball does 5 whopping damage! Woah!
  s_magic_stats[fb_id] = Spell(5.0f, 2.0f, fbs::MAGIC_TYPE::FIREBALL);

  uint32_t mm_id = any_enum(fbs::MAGIC_TYPE::MAGIC_MISSILE);
  s_magic_stats[mm_id] = Spell(3.0f, 0.5f, fbs::MAGIC_TYPE::MAGIC_MISSILE);
}

// Magic missles must come from a city.
bool magic::missile_requirements(const sf::Vector3i& location, uint32_t player_id) {
  // Verify that a players city is nearby 
  auto find_city = [player_id](const City& c) { 
    if (c.m_owner_id == player_id) return true; 
    return false;
  };

  // Dfs for for a city within two tiles of the target location.
  if (search::bfs_cities(location, 2, world_map::get_map(), find_city)) {
    return true;
  }

  std::cout << "A city is required within two tiles to cast magic missile" << std::endl;
  return false;
};

bool magic::fireball_requirements(const sf::Vector3i& location, uint32_t player_id) {
  auto find_wizard = [player_id](const Unit& u) {
    if (u.m_owner_id != player_id) return false;
    if (u.m_type != fbs::UNIT_TYPE::WIZARD) return false;
    return true;
  };

  if (search::bfs_units(location, 3, world_map::get_map(), find_wizard)) {
    return true;
  }

  std::cout << "A city is required within three tiles to cast fireball" << std::endl;
  return false;
};

void magic::reset() {
  for (auto& ms : s_magic_stats) {
    ms = Spell();
  }
}

void magic::cast(uint32_t player_id, fbs::MAGIC_TYPE type, const sf::Vector3i& location, bool cheat/*=false*/) {
  uint32_t utype = any_enum(type);
  float magic_cost = s_magic_stats[utype].m_cost;
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

  // Check that the spell satisfies all its requirements.
  if (!cheat) {
    bool success = false;
    switch (type) {
      case fbs::MAGIC_TYPE::FIREBALL:
        success = fireball_requirements(location, player_id);
        break;
      case fbs::MAGIC_TYPE::MAGIC_MISSILE:
        success = missile_requirements(location, player_id);
        break;
      case fbs::MAGIC_TYPE::UNKNOWN:
      default:
        std::cout << "Unkown spell type casted from player_id: " << player_id << std::endl;
        return;
    }
    if (!success) {
      std::cout << fbs::EnumNameMAGIC_TYPE(type) << " failed casting requirements." << std::endl;
    }
  }

  Tile* tile = world_map::get_tile(location);
  if (!tile) {
    std::cout << fbs::EnumNameMAGIC_TYPE(type) << " targeted invalid location." << std::endl;
    return;
  }

  switch (type) {
    case fbs::MAGIC_TYPE::FIREBALL:
      aoe_damage(player_id, type, *tile);
      break;
    case fbs::MAGIC_TYPE::MAGIC_MISSILE:
      damage_units(player_id, type, *tile);
      break;
    case fbs::MAGIC_TYPE::UNKNOWN:
      std::cout << "Unkown spell type casted from player_id: " << player_id << std::endl;
      return;
  }

  p->m_magic -= magic_cost;
}
