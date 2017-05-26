#pragma once

#include "simulation.h"

#include <Vector3.hpp>

namespace ai_shared {
  void approach(uint32_t unit_id, const sf::Vector3i& location); 
  bool attack_unit(uint32_t unit_id, uint32_t target_id);
  bool attack_city(uint32_t unit_id, uint32_t city_id);
  bool approach_unit(uint32_t unit_id, uint32_t target_id);
  bool approach_city(uint32_t unit_id, uint32_t target_id);
  bool pillage_improvement(uint32_t unit_id, uint32_t target_id);
  bool wander(uint32_t unit_id);
  bool approach_improvement(uint32_t unit_id, uint32_t target_id);
  sf::Vector3i get_random_coord();

  template<typename T>
  size_t simulate_step(const T& step, char* buffer, size_t buffer_len=256) {
    size_t bytes = serialize(buffer, buffer_len, step);
    simulation::process_step_from_ai(buffer, bytes);
    return bytes;
  }
}
