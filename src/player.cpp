#include "player.h"

#include <memory>
#include <vector>

#include "Vector3.hpp"
#include "city.h"
#include "improvement.h"
#include "unit.h"

Player::Player(uint32_t id)
    : m_id(id)
    , m_cities()
    , m_units() 
    , m_improvements()
    , m_discovered_players()
    , m_turn_state(TURN_TYPE::TURNACTIVE)
    , m_gold(0.0f)
    , m_science(0.0f)
    , m_magic(10.0) // Players should start with some magic
    , m_research(SCIENCE_TYPE::AGRICULTURE)
    , m_ai_type(AI_TYPE::UNKNOWN)
    , m_ai_state(nullptr)
    , m_omniscient(false)
{
}

bool Player::OwnsCity(uint32_t id) const {
  return m_cities.find(id) != m_cities.end();
}

bool Player::OwnsUnit(uint32_t id) const {
  return m_units.find(id) != m_units.end();
}

bool Player::OwnsImprovement(uint32_t id) const {
  return m_improvements.find(id) != m_improvements.end();
}

bool Player::DiscoveredPlayer(uint32_t id) const {
  return m_discovered_players.find(id) != m_discovered_players.end();
}

bool Player::DiscoveredCity(uint32_t id) const {
  return m_discovered_cities.find(id) != m_discovered_cities.end();
}

bool Player::DiscoveredScience(SCIENCE_TYPE st) const {
  uint32_t id = static_cast<uint32_t>(st);
  return m_discovered_science.find(id) != m_discovered_science.end();
}

namespace player {
  // List of players, player at index 0 will be player 1 ... index N player N - 1
  std::vector<Player*> s_players;

  void init(Player* p) {
    if (!p) return;

    // Remove unit when notified
    unit::sub_destroy([p](UnitFatality* uf) {
      if (p->m_id != uf->m_dead->m_owner_id) return;

      p->m_units.erase(uf->m_dead->m_id);
    });

    city::sub_raze_complete([p](const sf::Vector3i /*location*/, uint32_t id) {

      if (!p->OwnsCity(id)) {
        return;
      }
      p->m_cities.erase(id);
    });

    improvement::sub_destroy([p](const sf::Vector3i /*location*/, uint32_t id) {
      if (!p->OwnsImprovement(id)) {
        return;
      }
      p->m_improvements.erase(id);
    });

    p->m_available_research.push_back(static_cast<uint32_t>(SCIENCE_TYPE::AGRICULTURE));
    s_players.push_back(p);
  }
}

uint32_t player::create_human(const std::string& name) {
  uint32_t playerId = static_cast<uint32_t>(s_players.size());
  Player* p = new Player(playerId);
  p->m_ai_type = AI_TYPE::HUMAN;
  p->m_name = name;
  init(p);
  return playerId;
}

uint32_t player::create_ai(AI_TYPE type) {
  uint32_t playerId = static_cast<uint32_t>(s_players.size());
  Player* p = new Player(playerId);
  p->m_ai_type = type;
  p->m_name = get_ai_name(type);
  init(p);
  return playerId;
}

Player* player::get_player(uint32_t i) {
  if (i >= s_players.size()) {
    return nullptr;
  }

  return s_players[i];
}

size_t player::get_count() {
  return s_players.size();
}

ResourceUMap player::get_resources(uint32_t player_id) {
  ResourceUMap player_resources;

  Player* player = player::get_player(player_id);
  if (!player) return player_resources;

  for (auto& impv_id : player->m_improvements) {
    Improvement* impv = improvement::get_improvement(impv_id);
    if (!impv) continue;
    player_resources.add(impv->m_resource);
  }

  return player_resources;
}

void player::reset() {
  for (auto& player : s_players) {
    delete player;
  }
  s_players.clear();
}

void player::set_omniscient(uint32_t player_id) {
  Player* player = get_player(player_id);
  if (!player) return;

  player->m_omniscient = true;
}

void player::add_city(uint32_t player_id, uint32_t city_id) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }

  player->m_cities.insert(city_id);
}

void player::add_unit(uint32_t player_id, uint32_t unit_id) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }

  player->m_units.insert(unit_id);
}

void player::add_improvement(uint32_t player_id, uint32_t improvement_id) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }

  player->m_improvements.insert(improvement_id);
}

void player::add_discovered_player(uint32_t player_id, uint32_t other_player_id) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }

  player->m_discovered_players.insert(other_player_id);
}

void player::add_discovered_city(uint32_t player_id, uint32_t city_id) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }

  player->m_discovered_cities.insert(city_id);
}

bool player::all_players_turn_ended() {
  bool allPlayersReady = true;
  player::for_each_player([&allPlayersReady](Player& player) {
    allPlayersReady &= player.m_turn_state == TURN_TYPE::TURNCOMPLETED;
  });
  return allPlayersReady;
}

void player::for_each_player(std::function<void(Player& player)> operation) {
  for (auto p : s_players) {
    operation(*p);
  }
}

void player::for_each_player_unit(uint32_t player_id, std::function<void(Unit& unit)> operation) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }
  for (auto u : player->m_units) {
    Unit* unit = unit::get_unit(u);
    if (!unit) {
      continue;
    }
    operation(*unit);
  }
}

void player::for_each_player_city(uint32_t player_id, std::function<void(City& city)> operation) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }
  for (auto c : player->m_cities) {
    City* city = city::get_city(c);
    if (!city) {
      continue;
    }
    operation(*city);
  }
}

void player::for_each_player_improvement(uint32_t player_id, std::function<void(Improvement&)> operation) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }
  for (auto i : player->m_improvements) {
    Improvement* imprv = improvement::get_improvement(i);
    if (!imprv) {
      continue;
    }
    operation(*imprv);
  }
}

void player::for_each_player_meet(uint32_t player_id, std::function<void(Player& player)> operation) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }
  for (auto c : player->m_discovered_players) {
    Player* other = get_player(c);
    if (!other) {
      continue;
    }
    operation(*other);
  } 
}

void player::for_each_city_found(uint32_t player_id, std::function<void(City& city)> operation) {
  Player* player = get_player(player_id);
  if (!player) {
    return;
  }
  for (auto id : player->m_discovered_cities) {
    City* c = city::get_city(id);
    if (c) continue;
    operation(*c);
  }
}
