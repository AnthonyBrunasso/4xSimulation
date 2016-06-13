#pragma once

struct Step;

namespace simulation {
  void start();
  void kill();

  void process_step(Step* step);
  void process_step_from_ai(Step* step);
  void process_begin_turn();
  void process_end_turn();
}
