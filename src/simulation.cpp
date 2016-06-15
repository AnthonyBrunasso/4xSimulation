#include "simulation.h"

#include "search.h"
#include "step.h"
#include "units.h"
#include "city.h"
#include "format.h"
#include "world_map.h"
#include "hex.h"
#include "util.h"
#include "player.h"
#include "production.h"
#include "unit_definitions.h"
#include "improvements.h"
#include "game_types.h"
#include "terrain_yield.h"
#include "ai_barbarians.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace {
  // Contains the current execution step
  Step* s_current_step;
  // Units to move in the current step
  typedef std::vector<uint32_t> UnitMovementVector;
  UnitMovementVector s_units_to_move;
  // Units to fight in the current step
  std::vector<std::pair<uint32_t, uint32_t> > s_units_to_fight;

  // Used to count turns and index into player array
  uint32_t s_current_turn = 0;

  // Order of operations that should be checked after a step
  bool step_move(UnitMovementVector& units_to_move);
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
  void phase_science_done();

  bool step_move(UnitMovementVector& units_to_move) {
    bool movement = false;

    UnitMovementVector still_moving;
    for (auto unit_id : units_to_move) {
      // Make sure the unit continues to exist
      Unit* unit = units::get_unit(unit_id);
      if (!unit) {
        std::cout << "Dropping dead unit " << unit_id << " (id) from movement. " << std::endl;
        continue;
      }

      // Unit arrived at destination
      if (unit->m_path.empty()) {
        std::cout << "Path complete; dropping unit " << unit_id << " (id) from movement " << std::endl;
        continue;
      }

      // Unit is engaged in movement, but exhausted
      if (!unit->m_action_points) {
        still_moving.push_back(unit_id);
        continue;
      }

      // Advance unit forward by a single action point
      uint32_t moved = world_map::move_unit(unit_id, 1);
      if (moved) {
        // Check if the units owner has discovered another unit.
        auto found_other = [&unit](const Tile& tile) {
          Player* current = player::get_player(unit->m_owner_id);
          if (!current) return false;
          for (auto id : tile.m_unit_ids) {
            // If this player doesn't own the unit return true.
            bool found = false;
            if (!current->OwnsUnit(id)) {
              // Get that unit
              Unit* other = units::get_unit(id);
              current->m_discovered_players.insert(other->m_owner_id); 
              // Don't early out, more than one unit *can* be on the tile.
              found = true;
            }
          }
          return false;
        };
        search::bfs(unit->m_location, 3, world_map::get_map(), found_other);
        movement = true;
        unit->m_action_points -= moved;

        still_moving.push_back(unit_id);
      }
      // else
      // there is an unforseen problem with the unit's path
    }

    units_to_move.swap(still_moving);

    return movement;
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
      
      if (!unit->m_action_points) {
        std::cout << "Unit " << unit->m_unique_id << " (id) is too exhausted to initiate combat. " << std::endl;
        continue;
      }

      // If combat occurs deduct action points from the initiator
      if (units::combat(pair.first, pair.second)) {
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
      TerrainYield t = cityInstance.DumpYields(true);
      cityInstance.Simulate(t);
      Player* player = player::get_player(cityInstance.m_owner_id);
      if (!player) return;
      player->m_gold += t.m_gold;
      player->m_science += t.m_science;
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

  void phase_queued_movement() {
    while (step_move(s_units_to_move)) {

    }
  }

  void phase_science_done() {

  }

  void execute_construction() {
    ConstructionStep* construction_step = static_cast<ConstructionStep*>(s_current_step);
    Player* player = player::get_player(construction_step->m_player);
    if (!player) {
      std::cout << "Invalid player index" << std::endl;
      return;
    }
    City* city = city::get_city(construction_step->m_city_id);
    if(!city) {
      std::cout << "City does not exist" << std::endl;
      return;
    }
    if (!player->OwnsCity(construction_step->m_city_id)) {
      std::cout << "Player does not own city" << std::endl;
      return;
    }

    if (!construction_step->m_cheat) {
      city->GetConstruction()->Add(production::id(construction_step->m_production_id));
      return;
    }
 
    city->GetConstruction()->Cheat(production::id(construction_step->m_production_id));
  }

  void execute_colonize() {
    ColonizeStep* colonize_step = static_cast<ColonizeStep*>(s_current_step);
    Player* player = player::get_player(colonize_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    uint32_t id = city::create(BUILDING_TYPE::TOWN, colonize_step->m_location, colonize_step->m_player);
    if (!id) {
      // Colonization failed.
      return;
    }
    player::add_city(colonize_step->m_player, id);
    std::cout << "player " << player->m_name << " colonized at: " << format::vector3(colonize_step->m_location) << std::endl;
  }

  void execute_improve() {
    ImproveStep* improve_step = static_cast<ImproveStep*>(s_current_step);
    Player* player = player::get_player(improve_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    uint32_t id = improvement::create(static_cast<IMPROVEMENT_TYPE>(improve_step->m_improvement_type), improve_step->m_location, improve_step->m_player);
    std::cout << "adding improvement to: " << improve_step->m_player << std::endl;
    player::add_improvement(improve_step->m_player, id);
  }

  void execute_harvest() {
    HarvestStep* harvest_step = static_cast<HarvestStep*>(s_current_step);
    Player* player = player::get_player(harvest_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    Tile* tile = world_map::get_tile(harvest_step->m_destination);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    if (terrain_yield::is_harvested(harvest_step->m_destination)) {
      terrain_yield::remove_harvest(harvest_step->m_destination);
      return;
    }
    City* city = city::nearest_city(harvest_step->m_destination);
    if (!city) {
      std::cout << "No valid city found" << std::endl;
      return; 
    }
    if (!player->OwnsCity(city->m_id)) {
      std::cout << "Terrain is not owned by this player" << std::endl;
      return;
    }
    if (terrain_yield::add_harvest(harvest_step->m_destination, city)) {
      std::cout << "City (" << city->m_id << ") is now harvesting from " << format::tile(*tile) << std::endl;
      return;
    }
}

  void execute_tile_mutator() {
    TileMutatorStep* tile_mutator_step = static_cast<TileMutatorStep*>(s_current_step);
    Tile* tile = world_map::get_tile(tile_mutator_step->m_destination);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    tile->m_path_cost = tile_mutator_step->m_movement_cost;
  }

  void execute_specialize() {
    SpecializeStep* specialize_step = static_cast<SpecializeStep*>(s_current_step);
    City* city = city::get_city(specialize_step->m_city_id);
    if (!city) {
      std::cout << "Invalid city" << std::endl;
      return;
    }
    Player* player = player::get_player(specialize_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    if (!player->OwnsCity(city->m_id)) {
      std::cout << "Player does not own city" << std::endl;
      return;
    }
    if (!city->CanSpecialize()) {
      std::cout << "City is not ready for specialization." << std::endl;
    }
    if (city->SetSpecialization(static_cast<TERRAIN_TYPE>(specialize_step->m_terrain_type))) {
      std::cout << "City has specialized in " << get_terrain_name(static_cast<TERRAIN_TYPE>(specialize_step->m_terrain_type)) << std::endl;
    }
  }

  void execute_resource_mutator() {
    ResourceMutatorStep* resource_mutator_step = static_cast<ResourceMutatorStep*>(s_current_step);
    Tile* tile = world_map::get_tile(resource_mutator_step->m_destination);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    Resource newResource(static_cast<RESOURCE_TYPE>(resource_mutator_step->m_type), resource_mutator_step->m_quantity);
    tile->m_resources.push_back(newResource);
  }

  void execute_kill() {
    KillStep* kill_step = static_cast<KillStep*>(s_current_step);
    units::destroy(kill_step->m_unit_id);
    std::cout << kill_step->m_unit_id << " (id) has been slain." << std::endl;
  }

  void execute_spawn() {
    SpawnStep* spawn_step = static_cast<SpawnStep*>(s_current_step);
    Player* player = player::get_player(spawn_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    units::create(static_cast<UNIT_TYPE>(spawn_step->m_unit_type), spawn_step->m_location, spawn_step->m_player);
  }

  Unit* generate_path() {
    MoveStep* move_step = static_cast<MoveStep*>(s_current_step);
    Player* player = player::get_player(move_step->m_player);
    Unit* unit = units::get_unit(move_step->m_unit_id);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return nullptr;
    }
    if (!unit) {
      std::cout << "Unit: " << move_step->m_unit_id << " does not exist." << std::endl;
      return nullptr;
    }
    if (player && !player->OwnsUnit(move_step->m_unit_id)) {
      std::cout << "Player does not own unit" << std::endl;
    }
    if (world_map::get_tile(move_step->m_destination) == nullptr) {
      std::cout << "Invalid destination: " << format::vector3(move_step->m_destination) << std::endl;
      return nullptr;
    }

    std::vector<sf::Vector3i> path; 
    // Run pathfinding to location
    search::path_to(unit->m_location, move_step->m_destination, world_map::get_map(), path);
    if (!path.empty()) {
      path.erase(path.begin());
    }
    // Set path
    units::set_path(unit->m_unique_id, path);
    return unit;
  }

  void execute_move() {
    std::cout << "Executing immediate movement " << std::endl;
    // Just set where the unit needs to move and add it to a list. The actual move will happen in the move phase

    Unit* unit = generate_path();
    if (!unit) return;
    // Execute on path
    UnitMovementVector units_to_move;
    units_to_move.push_back(unit->m_unique_id);
    while (step_move(units_to_move)) {

    }

    // Queue it for continued movement, unit will remove itself if it is done moving
    s_units_to_move.push_back(unit->m_unique_id);
  }

  void execute_queue_move() {
    Unit* unit = generate_path();
    s_units_to_move.push_back(unit->m_unique_id);
  }

  void execute_add_player() {
    AddPlayerStep* player_step = static_cast<AddPlayerStep*>(s_current_step);
    uint32_t player_id = player::create(player_step->ai_type, player_step->m_name);
    if (player_step->ai_type != AI_TYPE::UNKNOWN) {
      switch (player_step->ai_type) {
        case AI_TYPE::BARBARIAN:
          barbarians::set_player_id(player_id);
          break;
        case AI_TYPE::UNKNOWN:
        default:
          break;
      }
    }
  }

  void execute_attack() {
    // Add a fight to the list to be executed in the combat phase
    AttackStep* attack_step = static_cast<AttackStep*>(s_current_step);
    Player* player = player::get_player(attack_step->m_player);
    if (!player) {
      std::cout << "Invalid player for attack action" << std::endl;
      return;
    }
    if (!player->OwnsUnit(attack_step->m_attacker_id)) {
      std::cout << "Player does not own attacking unit" << std::endl;
      return;
    }
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

    std::cout << format::combat_stats(unit->m_combat_stats) << std::endl;
  }
}

void simulation::start() {
  // Magic numbers
  sf::Vector3i start;
  world_map::build(start, 10);
  // Setup unit definitions
  unit_definitions::initialize();
  world_map::load_file("marin.dat");
}

void simulation::kill() {
  units::clear();
  city::clear();
}

uint32_t simulation::get_turn() {
  return s_current_turn;
}

void player_notifications(uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) {
    return;
  }
  if (player->m_turn_state == TURN_TYPE::TURNCOMPLETED) {
    return;
  }
  std::cout << std::endl << "Active player is " << player_id << std::endl;
  player::for_each_player_city(player_id, [] (City& c) {
    c.DoNotifications();
  });
  player::for_each_player_unit(player_id, [] (Unit& u) {
    if (u.m_path.empty()) {
      std::cout << "  " << u.m_unique_id;
    }
  });
  std::cout << "  <--- Idle units" << std::endl;
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
      execute_construction();
      break;
    case COMMAND::DISCOVER:
      break;
    case COMMAND::HARVEST:
      execute_harvest();
      break;
    case COMMAND::IMPROVE:
      execute_improve();
      break;
    case COMMAND::TILE_MUTATOR:
      execute_tile_mutator();
      break;
    case COMMAND::RESOURCE_MUTATOR:
      execute_resource_mutator();
      break;
    case COMMAND::KILL:
      execute_kill();
      break;
    case COMMAND::MOVE:
      execute_move();
      break;
    case COMMAND::QUEUE_MOVE:
      execute_queue_move();
      break;
    case COMMAND::PURCHASE:
      break;
    case COMMAND::SPECIALIZE:
      execute_specialize();
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
    case COMMAND::BARBARIAN_TURN:
      // Process the barbarian turn.
      barbarians::pillage_and_plunder();
      return;
    default:
      break;
  }

  // Process potential outputs of steps
  step_negotiate();
  step_discoever();
  step_combat();
  step_death();
  step_raze();
  step_verterancy();
  step_resource();
}

// Same as regular step, just deletes step on behalf of ai when finished.
void simulation::process_step_from_ai(Step* step) {
  simulation::process_step(step);
  delete step;
}

void grant_improvement_resources(const Improvement& improvement) {
  Tile* tile = world_map::get_tile(improvement.m_location);
  if (!tile) {
    return;
  }

  Player* player = player::get_player(improvement.m_owner_id);
  if (!player) {
    return;
  }

  // This will need some more work. This will cause all resources on a tile to be granted
  // to the player. Perhaps we will need to check the type of resource/improvement if there are 
  // multiple resource on the tile for instance. 
  for (auto resource : tile->m_resources) {
    player->m_resources.add(resource); 
  }
}

void simulation::process_begin_turn() {
  BeginStep* begin_step = static_cast<BeginStep*>(s_current_step);

  bool allPlayersReady = true;
  player::for_each_player([&allPlayersReady](Player& player) {
    allPlayersReady &= player.m_turn_state == TURN_TYPE::TURNCOMPLETED;
  });

  if (!allPlayersReady) {
    std::cout << "Not all players ready for next turn. " << std::endl;
    return;
  }

  // Apply changes
  phase_queued_movement();
  phase_city_growth();
  phase_science_progression();
  phase_diplomatic_progression();
  phase_spawn_units();
  phase_spawn_buildings();

  // Provide turn feedback
  phase_global_events();
  phase_science_done();

  // Increment turn counter
  ++s_current_turn;

  // Run improvement specific work. Example: Grant players reources for their improved tiles.
  improvement::for_each_improvement([](const Improvement& improvement) {
    switch(improvement.m_type) {
    case IMPROVEMENT_TYPE::UNKNOWN:
      break;
    case IMPROVEMENT_TYPE::RESOURCE:
      grant_improvement_resources(improvement); 
      break;
    default:
      break;
    }
  });

  // Each player state -> Playing
  player::for_each_player([](Player& player) {
    player.m_turn_state = TURN_TYPE::TURNACTIVE;
  });
  std::cout << std::endl << "Beginning turn #" << s_current_turn << std::endl;

  phase_restore_actions();
  player_notifications(begin_step->m_active_player);
}

void simulation::process_end_turn() {
  EndTurnStep* end_step = static_cast<EndTurnStep*>(s_current_step);

  Player* player = player::get_player(end_step->m_player);

  if (!player) {
    return;
  }

  player->m_turn_state = TURN_TYPE::TURNCOMPLETED;
  player_notifications(player->m_id);
}
