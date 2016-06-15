#include "ai_evaluations.h"

#include "player.h"
#include "city.h"
#include "ai_barbarians.h"
#include "tile.h"
#include "search.h"
#include "world_map.h"
#include "units.h"

#include <iostream>

float NeedsColonize::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Needs colonization evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating colonization state." << std::endl;
  if (player->m_cities.empty()) {
    std::cout << player->m_name << " needs to colonize." << std::endl;
    return threshold + 1.0f;
  }
  else {
    return threshold - 1.0f;
  }
}


float NeedsProduce::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Needs production evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating production state." << std::endl;
  for (auto id : player->m_cities) {
    City* city = city::get_city(id);
    if (!city) continue;
    // If the cities production queue is empty the player needs production.
    if (!city->IsConstructing()) {
      std::cout << player->m_name << " needs production." << std::endl;
      return threshold + 1.0f;
    }
  }

  // Player doesn't have an available city for production.
  return threshold - 1.0f;
}

float HasUnits::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Has units evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  std::cout << player->m_name << " evaluating its unit state." << std::endl;
  if (!player->m_units.empty()) {
    return threshold + 1.0f;
  }

  std::cout << player->m_name << " has no units." << std::endl;
  return threshold - 1.0f;
}

float discovered_cities_barbarian(Player* player, float threshold) {
  std::cout << player->m_name << " evaluating nearby cities." << std::endl;
  if (!barbarians::discovered_city()) {
    return threshold - 1.0f;
  }
  return threshold + 1.0f;
}

float DiscoveredCities::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "Discovered city evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  switch (player->m_ai_type) {
    case AI_TYPE::BARBARIAN:
      return ::discovered_cities_barbarian(player, threshold);
    case AI_TYPE::HUMAN:
    default:
      std::cout << "DiscoveredCities evaluation not evaluated for: " << get_ai_name(player->m_ai_type) << std::endl;
      return NOOP_EVALUATION;
  }

  return NOOP_EVALUATION;
}

float discover_cities_barbarian(Player* player, float threshold) {
  std::cout << player->m_name << " looking for enemy cities." << std::endl;
  auto find_enemy_cities = [player](const Tile& tile) -> bool {
    if (!tile.m_city_id) {
      return false;
    }
    // Determine if this player owns the city.
    City* c = city::get_city(tile.m_city_id);
    if (!c) return false;
    if (c->m_owner_id == player->m_id) return false;
    std::cout << player->m_name << " found city: " << c->m_id << std::endl;
    return true;
  };
 
  float result = threshold - 1.0f;
  for (auto u : player->m_units) {
    Unit* unit = units::get_unit(u);
    if (!unit) continue;
    // Search for enemy cities around all units.
    if (search::bfs(unit->m_location, 3, world_map::get_map(), find_enemy_cities)) {
      result = threshold + 1.0f;
    }
  }

  return result;
}


float DiscoverCities::operator()(uint32_t player_id, float threshold) {
  Player* player = player::get_player(player_id);
  if (!player) {
    std::cout << "DiscoverCities evaluation found no player." << std::endl;
    return NOOP_EVALUATION;
  }

  switch (player->m_ai_type) {
  case AI_TYPE::BARBARIAN:
    return ::discover_cities_barbarian(player, threshold);
  case AI_TYPE::HUMAN:
  default:
    std::cout << "DiscoverCities evaluation not evaluated for: " << get_ai_name(player->m_ai_type) << std::endl;
    return NOOP_EVALUATION;
  }

  return NOOP_EVALUATION;
}
