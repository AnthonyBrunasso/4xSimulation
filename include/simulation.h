#pragma once

#include <cstddef>
#include <cstdint>

namespace fbs {
  class EndTurnStep;
};

namespace simulation {
  void kill();
  bool game_over();

  void reset();

  uint32_t get_turn();
  
  void process_step(const void* buffer, size_t buffer_len);
  void process_step_from_ai(const void* buffer, size_t buffer_len);
  void process_begin_turn();
  void process_end_turn(const fbs::EndTurnStep*);
}
