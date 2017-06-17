#pragma once

#include <stdint.h>

#include "Vector3.hpp"

#include "enum_generated.h"

namespace magic {
  // Sets attributes of spells.
  void initialize();
  void reset();
  void cast(uint32_t player_id, fbs::MAGIC_TYPE type, const sf::Vector3i& location, bool cheat=false);  
}
