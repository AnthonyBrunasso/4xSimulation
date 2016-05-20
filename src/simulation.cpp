#include "simulation.h"

#include "step.h"
#include "units.h"
#include "city.h"
#include "world_map.h"
#include "hex.h"
#include "util.h"
#include "player.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace {
  // Contains the current execution step
  Step* s_current_step;
  // Units to move in the current step
  std::vector<Unit*> s_units_to_move;
  // Units to fight in the current step
  std::vector<std::pair<uint32_t, uint32_t> > s_units_to_fight;
  // Used to count turns and index into player array
  uint32_t s_current_turn = 0;

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
        if (world_map::move_unit(unit->m_unique_id, 1)) {
          --unit->m_action_points;
        }
      }

      // If they have arrived at their destination they are done moving
      if (unit->m_path.empty()) {
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
    for (auto pair : s_units_to_fight) {
      Unit* unit = units::get_unit(pair.first);
      if (!unit) {
        continue;
      }

      if (unit->m_action_points) {
        units::combat(pair.first, pair.second);
        --unit->m_action_points;
      }
    }

    // Attacks should all complete in a single step?
    s_units_to_fight.clear();
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
    uint32_t id = city::create(colonize_step->m_location);
    player::add_city(colonize_step->m_player, id);
  }

  void execute_spawn() {
    SpawnStep* spawn_step = static_cast<SpawnStep*>(s_current_step);
    uint32_t id = units::create(static_cast<ENTITY_TYPE>(spawn_step->m_entity_type), spawn_step->m_location);
    player::add_unit(spawn_step->m_player, id);
  }

  void execute_move() {
    // Just set where the unit needs to move and add it to a list. The actual move will happen in the move phase
    MoveStep* move_step = static_cast<MoveStep*>(s_current_step);
    Unit* unit = units::get_unit(move_step->m_unit_id);
    if (!unit) {
      return;
    }
    units::set_path(move_step->m_unit_id, move_step->m_destination);
    s_units_to_move.push_back(unit);
  }

  void execute_add_player() {
    AddPlayerStep* player_step = static_cast<AddPlayerStep*>(s_current_step);
    player::create(player_step->m_name);
  }

  void execute_attack() {
    // Add a fight to the list to be executed in the combat phase
    AttackStep* attack_step = static_cast<AttackStep*>(s_current_step);
    s_units_to_fight.push_back(std::pair<uint32_t, uint32_t>(attack_step->m_attacker_id, attack_step->m_defender_id));
  }

  void execute_modify_stats() {
    UnitStatsStep* stats_step = static_cast<UnitStatsStep*>(s_current_step);
    Unit* unit = units::get_unit(stats_step->m_unit_id);
    if (!unit) {
      return;
    }
    unit->m_combat_stats.m_health = stats_step->m_health;
    unit->m_combat_stats.m_attack = stats_step->m_attack;
    unit->m_combat_stats.m_range = stats_step->m_range;
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
      execute_attack();
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
    case COMMAND::ADD_PLAYER:
      execute_add_player();
      return; // Special case, adding a player does not have output
    case COMMAND::MODIFY_UNIT_STATS:
      execute_modify_stats();
      return; // Modifying stats also does not have output
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
  phase_notifications();
  phase_spawn_units();
  phase_spawn_buildings();
  phase_science_done();

  // Increment turn counter
  ++s_current_turn;
}
