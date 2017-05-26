#pragma once

#include "Vector3.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

struct Step;

namespace fbs {
  class EndTurnStep;
};

namespace simulation {
  void start();
  void kill();

  void reset();

  uint32_t get_turn();
  
  void process_step(const void* buffer, size_t buffer_len);
  void process_step_from_ai(const void* buffer, size_t buffer_len);
  void process_begin_turn();
  void process_end_turn(const fbs::EndTurnStep*);
}
