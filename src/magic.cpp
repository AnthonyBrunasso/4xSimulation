#include "magic.h"

#include "tile.h"
#include "world_map.h"
#include "util.h"
#include "unit.h"
#include "search.h"
#include "player.h"
#include "world_map.h"
#include "city.h"

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

  typedef std::vector<std::function<bool(uint32_t, const sf::Vector3i&)> > Requirements;
  typedef std::unordered_map<uint32_t, Requirements> RequirementMap;
  RequirementMap s_requirements;

  void damage_units(uint32_t casting_player, MAGIC_TYPE type, const sf::Vector3i& location) {
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      std::cout << get_magic_name(type) << " targeted invalid location." << std::endl;
      return;
    }
  
    float dmg = s_magic_stats[util::enum_to_uint(type)].m_damage;
    for (auto id : tile->m_unit_ids) {
      // Rain ze fire.
      std::cout << "Casting " << get_magic_name(type) << " upon unit " << id << std::endl;
      // If a unit died, jump out since tile->m_unit_ids will mutate.
      if (unit::damage(id, casting_player, dmg)) break;
    }
  }
}

void magic::initialize() {
  uint32_t fb_id = util::enum_to_uint(MAGIC_TYPE::FIREBALL);
  // Fireball does 5 whopping damage! Woah!
  s_magic_stats[fb_id] = Spell(5.0f, 2.0f, MAGIC_TYPE::FIREBALL);

  uint32_t mm_id = util::enum_to_uint(MAGIC_TYPE::MAGIC_MISSLE);
  s_magic_stats[mm_id] = Spell(3.0f, 0.5f, MAGIC_TYPE::MAGIC_MISSLE);

  // Magic missles must come from a city.
  auto missle_requirements = [](uint32_t player_id, const sf::Vector3i& location) {
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

  auto fireball_requirements = [](uint32_t player_id, const sf::Vector3i& location) {
    auto find_wizard = [player_id](const Unit& u) {
      if (u.m_owner_id != player_id) return false;
      if (u.m_type != UNIT_TYPE::WIZARD) return false;
      return true;
    };

    if (search::bfs_units(location, 3, world_map::get_map(), find_wizard)) {
      return true;
    }

    std::cout << "A city is required within three tiles to cast fireball" << std::endl;
    return false;
  };

  s_requirements[mm_id].push_back(missle_requirements);
  s_requirements[fb_id].push_back(fireball_requirements);
}

void magic::cast(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location, bool cheat/*=false*/) {
  uint32_t utype = util::enum_to_uint(type);
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
  if (!cheat && s_requirements.find(utype) != s_requirements.end()) {
    for (auto req : s_requirements[utype]) {
      if (!req(player_id, location)) {
        std::cout << get_magic_name(type) << " does not meet requirements" << std::endl;
        return;
      }
    }
  }

  switch (type) {
    case MAGIC_TYPE::FIREBALL:
      damage_units(player_id, type, location);
      break;
    case MAGIC_TYPE::MAGIC_MISSLE:
      damage_units(player_id, type, location);
      break;
    case MAGIC_TYPE::UNKNOWN:
      std::cout << "Unkown spell type casted from player_id: " << player_id << std::endl;
      return;
  }

  p->m_magic -= magic_cost;
}
