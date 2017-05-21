#pragma once
#include <stdint.h>

namespace scenario_arena {
  bool active();
  uint32_t get_score(uint32_t player_id);

  void start();
  void process();

  void reset();
}
