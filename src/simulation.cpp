#include "simulation.h"

namespace {
  void step_move();
  void step_negotiate();
  void step_discoever();
  void step_combat();
  void step_death();
  void step_raze();
  void step_verterancy();
  void step_resource();

  void phase_city_growth();
  void phase_science_progression();
  void phase_diplomatic_progression();
  void phase_global_events();

  void phase_spawn_units();
  void phase_spawn_buildings();
  void phase_notifications();
  void phase_science_done();
}

void simulation::process_step(Step* step) {
  // If step is begin turn
  process_begin_turn();

  

  // If step is end turn
  process_end_turn();
}

void simulation::process_begin_turn() {
  phase_city_growth();
  phase_science_progression();
  phase_diplomatic_progression();
  phase_global_events();
}

void simulation::process_end_turn() {
  phase_spawn_units();
  phase_spawn_buildings();
  phase_notifications();
  phase_science_done();

  // Write output file
}