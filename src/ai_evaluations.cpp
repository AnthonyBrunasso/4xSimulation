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
  if (player->m_discovered_cities.empty()) {
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
