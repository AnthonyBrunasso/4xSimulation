#include "simulation.h"

#include "step.h"
#include "units.h"
#include "city.h"
#include "world_map.h"

#include <iostream>

namespace {
  Step* s_current_step;

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
  void phase_spawn_units();     // Spawn that occurs from construction countdown, etc
  void phase_spawn_buildings();
  void phase_notifications();
  void phase_science_done();

  void process_spawn();         // Immediate spawn 

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
    city::for_each_city([](City& cityInstance) { 
      cityInstance.Simulate();
    });
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

  void execute_colonize() {
    ColonizeStep* colonize_step = static_cast<ColonizeStep*>(s_current_step);
    units::destroy(colonize_step->m_entity_id);
    city::create(colonize_step->m_entity_id, colonize_step->m_location);
  }

  void execute_spawn() {
    SpawnStep* spawn_step = static_cast<SpawnStep*>(s_current_step);
    units::create(static_cast<ENTITY_ID>(spawn_step->m_entity_id), spawn_step->m_location);
  }
}

void simulation::start() {
  // Magic numbers
  sf::Vector3i start(0, 0, 0);
  world_map::build(start, 10);
}

void simulation::kill() {
  units::clear();
  city::clear();
}

void simulation::process_step(Step* step) {
  // Save a pointer to the current step
  s_current_step = step;

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
      execute_colonize();
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
    case COMMAND::SPAWN:
      execute_spawn();
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
  std::cout << "Beginning turn..." << std::endl;
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
