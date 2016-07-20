#pragma once

#include "Vector3.hpp"

#include <cstdint>
#include <vector>

struct Step;

namespace simulation {
  void start();
  void kill();

  uint32_t get_turn();
  
  void process_step(Step* step);
  void process_step_from_ai(Step* step);
  void process_begin_turn();
  void process_end_turn();
}
