#include "simulation.h"

#include "step.h"
#include "units.h"
#include "city.h"
#include "world_map.h"
#include "hex.h"
#include "util.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace {
  // Contains the current execution step
  Step* s_current_step;

  // Units to move in the current step, likely size 1 or 0
  std::vector<Unit*> s_units_to_move;

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
  void phase_restore_actions();

  // Order of operations when a turn begins
  void phase_spawn_units();     // Spawn that occurs from construction countdown, etc
  void phase_spawn_buildings();
  void phase_notifications();
  void phase_science_done();

  void process_spawn();         // Immediate spawn 

  void step_move() {
    std::vector<Unit*> done_moving;
    for (auto unit : s_units_to_move) {
      // Advance unit forward by a single action point
      if (unit->m_action_points) {
        if (units::move(unit->m_unique_id, 1)) {
          --unit->m_action_points;
        }
      }

      // If they have arrived at their destination they are done moving
      if (!unit->m_path.size()) {
        done_moving.push_back(unit);
      }
    }

    // Remove all units that are finished moving
    for (auto unit : done_moving) {
      auto findIt = std::find(s_units_to_move.begin(), s_units_to_move.end(), unit);
      if (findIt != s_units_to_move.end()) {
        s_units_to_move.erase(findIt);
      }
    }
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

  void phase_restore_actions() {
    units::replenish_actions();
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
    units::destroy(colonize_step->m_unit_id);
    city::create(colonize_step->m_location);
  }

  void execute_spawn() {
    SpawnStep* spawn_step = static_cast<SpawnStep*>(s_current_step);
    units::create(static_cast<ENTITY_TYPE>(spawn_step->m_entity_type), spawn_step->m_location);
  }

  void execute_move() {
    // Just set where the unit needs to move and add it to a list. The actual move will happen in the move phase
    MoveStep* move_step = static_cast<MoveStep*>(s_current_step);
    Unit* unit = units::get_unit(move_step->m_unit_id);
    if (!unit) {
      return;
    }

    // Set the destination of the unit and queue it to move
    // TODO: Change this to a search algorithm like A* that weights tiles, for now, assume all tiles can be pathed
    hex::cubes_on_line(util::to_vector3f(unit->m_location), 
      util::to_vector3f(move_step->m_destination), 
      unit->m_path);

    std::cout << "Path size: " << unit->m_path.size() << std::endl;

    // TODO: When pathing algorithm built deal with not removing the first element on each move
    if (unit->m_path.size()) {
      unit->m_path.erase(unit->m_path.begin());
    }

    s_units_to_move.push_back(unit);
  }
}

void simulation::start() {
  // Magic numbers
  sf::Vector3i start;
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
      execute_move();
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
  phase_restore_actions();
}

void simulation::process_end_turn() {
  phase_spawn_units();
  phase_spawn_buildings();
  phase_notifications();
  phase_science_done();

  // Write output file
}
