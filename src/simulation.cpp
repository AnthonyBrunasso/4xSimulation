#include "simulation.h"

#include "step.h"
#include "world_map.h"

namespace {
  // Order of operations that should be checked after a step
  void step_move();
  void step_negotiate();
  void step_discoever();
  void step_combat();
  void step_death();
  void step_raze();
  void step_verterancy();
  void step_resource();

  // Order of operations when a turn ends
  void phase_city_growth();
  void phase_science_progression();
  void phase_diplomatic_progression();
  void phase_global_events();

  // Order of operations when a turn begins
  void phase_spawn_units();
  void phase_spawn_buildings();
  void phase_notifications();
  void phase_science_done();

  void step_move() {

  }

  void step_negotiate() {

  }

  void step_discoever() {

  }

  void step_combat() {

  }

  void step_death() {

  }

  void step_raze() {

  }

  void step_verterancy() {

  }

  void step_resource() {

  }

  void phase_city_growth() {

  }

  void phase_science_progression() {

  }

  void phase_diplomatic_progression() {

  }

  void phase_global_events() {

  }

  void phase_spawn_units() {

  }

  void phase_spawn_buildings() {

  }

  void phase_notifications() {

  }

  void phase_science_done() {

  }
}

void simulation::start() {
  // Magic numbers
  sf::Vector3i start(0, 0, 0);
  world_map::build(start, 10);
}

void simulation::process_step(Step* step) {
  // Process the step
  switch (step->m_command) {
    case COMMAND::QUIT:
      break;
    case COMMAND::BEGIN_TURN:
      process_begin_turn();
      break;
    case COMMAND::END_TURN:
      process_end_turn();
      break;
    case COMMAND::ATTACK:
      break;
    case COMMAND::COLONIZE:
      break;
    case COMMAND::CONSTRUCT:
      break;
    case COMMAND::DISCOVER:
      break;
    case COMMAND::IMPROVE:
      break;
    case COMMAND::KILL:
      break;
    case COMMAND::MOVE:
      break;
    case COMMAND::PURCHASE:
      break;
    case COMMAND::SELL:
      break;
    default:
      break;
  }

  // Process potential outputs of steps
  step_move();
  step_negotiate();
  step_discoever();
  step_combat();
  step_death();
  step_raze();
  step_verterancy();
  step_resource();
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