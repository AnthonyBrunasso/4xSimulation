#pragma once

#include <cstdint>
#include <vector>

namespace barbarians {
  void initialize();
  void reset();
  // Lets the barbarians execute their turn. 
  void pillage_and_plunder(uint32_t player_id);
  // Set the barbarians player id.
  void set_player_id(uint32_t player_id);
  // Disable barbarian macro behavior
  void disable_macro_behavior();
}
