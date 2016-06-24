#pragma once

#include "game_types.h"
#include "Vector3.hpp"

namespace magic {
  // Sets attributes of spells.
  void initialize();
  void cast(uint32_t player_id, MAGIC_TYPE type, const sf::Vector3i& location, bool cheat=false);  
}
