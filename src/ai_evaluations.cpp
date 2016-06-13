#include "ai_evaluations.h"

#include "player.h"

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
