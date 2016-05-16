#pragma once

struct Step;

namespace simulation {
  void start();
  void kill();

  void process_step(Step* step);
  void process_begin_turn();
  void process_end_turn();
}