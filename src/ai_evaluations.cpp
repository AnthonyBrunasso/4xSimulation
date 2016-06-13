#include "ai_evaluations.h"

#include "player.h"
#include "city.h"

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
