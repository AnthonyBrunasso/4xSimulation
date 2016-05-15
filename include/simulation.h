#pragma once

class Step;

namespace simulation {

  void process_step(Step* step);
  void process_begin_turn();
  void process_end_turn();
  
}