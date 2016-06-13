#include "player.h"
#include "units.h"
#include "city.h"
#include "improvements.h"

#include <iostream>
#include <vector>

Player::Player(uint32_t id, const std::string& name) 
    : m_id(id)
    , m_name(name)
    , m_cities()
    , m_units() 
    , m_improvements()
    , m_turn_state(TURN_TYPE::TURNACTIVE)
    , m_resources()
{
  // Remove unit when notified
  units::sub_destroy([this](const sf::Vector3i /*location*/, uint32_t id) {
    if (!this->OwnsUnit(id)) {
      return;
    }
    this->m_units.erase(id);
  });

  city::sub_raze([this](const sf::Vector3i /*location*/, uint32_t id) {
    if (!this->OwnsCity(id)) {
      return;
    }
    this->m_cities.erase(id);
  });

  improvement::sub_destroy([this](const sf::Vector3i /*location*/, uint32_t id) {
    if (!this->OwnsImprovement(id)) {
      return;
    }
    this->m_improvements.erase(id);
  });
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

namespace {
  // List of players, player at index 0 will be player 1 ... index N player N - 1
  std::vector<Player*> s_players;
}

uint32_t player::create(const std::string& name) {
  uint32_t playerId = static_cast<uint32_t>(s_players.size());
  s_players.push_back(new Player(playerId, name));
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

void player::for_each_player(std::function<void(Player& player)> operation) {
  for (auto p : s_players) {
    operation(*p);
  }
}
