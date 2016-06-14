#pragma once

#include <cstdint>
#include <vector>

namespace barbarians {
  void initialize();
  void destroy();
  // Lets the barbarians execute their turn. 
  void pillage_and_plunder();
  // Set the barbarians player id.
  void set_player_id(uint32_t player_id);
  void discover_city(uint32_t city_id);
  bool discovered_city();

  const std::vector<uint32_t> get_discovered_cities();
}
