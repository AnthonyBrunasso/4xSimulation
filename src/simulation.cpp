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
#include "unique_id.h"
#include "improvements.h"
#include "game_types.h"
#include "terrain_yield.h"
#include "ai_barbarians.h"
#include "science.h"
#include "magic.h"
#include "custom_math.h"
#include "status_effect.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace simulation {
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
  bool step_move(UnitMovementVector& units_to_move, uint32_t player_id);
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

  bool step_move(UnitMovementVector& units_to_move, uint32_t player_id) {
    bool movement = false;

    UnitMovementVector still_moving;
    for (auto unit_id : units_to_move) {
      // Make sure the unit continues to exist
      Unit* unit = units::get_unit(unit_id);
      if (!unit) {
        std::cout << "Dropping dead unit " << unit_id << " (id) from movement. " << std::endl;
        continue;
      }

      // Only move units for the given player.
      if (unit->m_owner_id != player_id) {
        still_moving.push_back(unit_id);
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
          if (tile.m_city_id) {
            City* c = city::get_city(tile.m_city_id);
            // If the player doesn't own the city and has not already discovered it.
            if (c && c->m_owner_id != current->m_id && !current->DiscoveredCity(tile.m_city_id)) {
              std::cout << current->m_name << " discoverd city: " << c->m_id << std::endl;
              current->m_discovered_cities.insert(c->m_id);
            }
          }
          for (auto id : tile.m_unit_ids) {
            // If this player doesn't own the unit return true.
            if (!current->OwnsUnit(id)) {
              // Get that unit
              Unit* other = units::get_unit(id);
              // Already discovered the player.
              if (current->DiscoveredPlayer(other->m_owner_id)) return false;
              Player* found_player = player::get_player(other->m_owner_id);
              if (!found_player) return false;
              std::cout << current->m_name << " discovered " << found_player->m_name << std::endl;
              current->m_discovered_players.insert(found_player->m_id); 
              found_player->m_discovered_players.insert(current->m_id);
            }
          }
          // Search every tile.
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

      Unit* defender = units::get_unit(pair.second);
      if (!defender) {
        continue;
      }
      
      if (!unit->m_action_points) {
        std::cout << "Unit " << unit->m_unique_id << " (id) is too exhausted to initiate combat. " << std::endl;
        continue;
      }

      // Attacker faces defender on combat initiation.
      units::change_direction(pair.first, defender->m_location);

      // If combat occurs deplete action points from the initiator
      if (units::combat(pair.first, pair.second)) {
        unit->m_action_points = 0;
      }

      // Defender turns to face attacker after combat.
      units::change_direction(pair.second, unit->m_location);
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
    auto city_func = [](City& cityInstance, Player& player) { 
      TerrainYield t = cityInstance.DumpYields();
      std::cout << t << std::endl;
      cityInstance.Simulate(t);
      player.m_gold += t.m_gold;
      player.m_science += t.m_science;
      player.m_magic += t.m_magic;
    };
    auto player_func = [city_func](Player& player) {
      player::for_each_player_city(player.m_id,
        [city_func, &player](City& c) { city_func(c, player); });
    };
    // Important: for_each_player is an ordered std::set
    // We must process players in order for consistent simulation results
    player::for_each_player(player_func);
  }

  void phase_science_progression() {
    player::for_each_player([] (Player& player) {
      ScienceNode* sn = science::Science(player.m_research);
      if (!sn) return;
      float req = science::research_cost(sn);
      if (player.m_science >= req) {
        std::cout << "Player " << player.m_id << " has discovered the science of " << get_science_name(player.m_research) << std::endl;
        player.m_science -= req;
        science::research_complete(player.m_id, sn);
        player.m_research = SCIENCE_TYPE::UNKNOWN;
      }
    });

  }

  void phase_diplomatic_progression() {

  }

  void phase_global_events() {

  }

  void phase_restore_actions() {
    units::replenish_actions();
    auto replentish_cities = [](City& c) {
      c.m_defenses_used = false;
    };
    city::for_each_city(replentish_cities);
  }

  void phase_spawn_units() {

  }

  void phase_spawn_buildings() {

  }

  void phase_queued_movement(uint32_t player_id) {
    while (step_move(s_units_to_move, player_id)) {

    }
  }

  void science_notifications() {
    player::for_each_player([] (Player& player) {
      if (player.m_research == SCIENCE_TYPE::UNKNOWN) {
        std::cout << "You require a new research task." << std::endl;
      }
    });
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
    CONSTRUCTION_TYPE t(production::id(construction_step->m_production_id));

    if (!construction_step->m_cheat) {
      city->GetConstruction()->Add(t);
      return;
    }
 
    city->Purchase(production::id(construction_step->m_production_id));
  }

  void execute_colonize() {
    ColonizeStep* colonize_step = static_cast<ColonizeStep*>(s_current_step);
    Player* player = player::get_player(colonize_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    bool too_close = false;
    search::bfs(colonize_step->m_location, 3, world_map::get_map(), 
      [&too_close](const Tile& tile) {
      if (tile.m_city_id != unique_id::INVALID_ID) {
        std::cout << "Colonization failed: City (" << tile.m_city_id << ") is too close." << std::endl;
        too_close = true;
        return true;
      }
      return false;
    });
    if (too_close) return;

    uint32_t id = city::create(BUILDING_TYPE::TOWN, colonize_step->m_location, colonize_step->m_player);
    if (!id) {
      // Colonization failed.
      return;
    }
    // If colonization succeeded there is a worker on the tile, destroy it.
    player::add_city(colonize_step->m_player, id);
    std::cout << "player " << player->m_name << " colonized city (" << id << ") at: " << format::vector3(colonize_step->m_location) << std::endl;
    Tile* t = world_map::get_tile(colonize_step->m_location);
    for (auto uid : t->m_unit_ids) {
      Unit* u = units::get_unit(uid);
      if (!u) continue;
      // Consume the unit that built the city.
      if (u->m_unit_type == UNIT_TYPE::WORKER && u->m_owner_id == player->m_id) {
        units::destroy(u->m_unique_id);
      }
    }
  }

  void execute_improve() {
    ImproveStep* improve_step = static_cast<ImproveStep*>(s_current_step);
    Player* player = player::get_player(improve_step->m_player);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    Tile* tile = world_map::get_tile(improve_step->m_location);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    size_t i = 0;
    Unit* unit = nullptr;
    uint32_t unit_id = 0;
    for (; i < tile->m_unit_ids.size(); ++i) {
      unit_id = tile->m_unit_ids[i];
      unit = units::get_unit(unit_id);
      if (!unit) continue;
      if (!unit->m_action_points) continue;
      if (unit->m_unit_type != UNIT_TYPE::WORKER) continue;
      break;
    }

    if (!unit || unit->m_unit_type != UNIT_TYPE::WORKER) { 
      std::cout << "No worker is available to improve the tile" << std::endl;
      return;
    }

    RESOURCE_TYPE rt = static_cast<RESOURCE_TYPE>(improve_step->m_resource);
    i = 0;
    for (; i < tile->m_resources.size(); ++i) {
      if (tile->m_resources[i].m_type == rt) break;
    }
    if (i == tile->m_resources.size()) {
      std::cout << "Resource not available on this tile." << std::endl;
      return;
    }
    Resource& res = tile->m_resources[i];
    auto impv(static_cast<IMPROVEMENT_TYPE>(improve_step->m_improvement_type));
    if (!improvement::satisfies_requirements(res.m_type, impv, improve_step->m_location)) {
      return;
    }

    uint32_t pid = improve_step->m_player;
    sf::Vector3i loc = improve_step->m_location;

    auto end_turn_inject = [res, impv, pid, loc, unit_id]() {
      uint32_t id = improvement::create(res, impv, loc, pid);

      if (id) {
        Unit* u = units::get_unit(unit_id);
        if (!u) { 
          std::cout << "unit no longer exists." << std::endl;
          return;
        }
        std::cout << "adding improvement to player: " << pid << std::endl;
        player::add_improvement(pid, id);
        u->m_action_points = 0;
      } 
    };

    status_effect::inject_end(end_turn_inject);
    if (status_effect::create(STATUS_TYPE::CONSTRUCTING_IMPROVEMENT, loc)) {
      // Drain workers action points.
      unit->m_action_points = 0;
    }
  }

  void execute_grant() {
    GrantStep* grant_step = static_cast<GrantStep*>(s_current_step);
    SCIENCE_TYPE st = static_cast<SCIENCE_TYPE>(grant_step->m_science);
    science::research_complete(grant_step->m_player, science::Science(st));
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
      std::cout << "City (" << city->m_id << ") is now harvesting from " << get_terrain_name(tile->m_terrain_type) << "." << std::endl;
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
      return;
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
    Resource new_resource(static_cast<RESOURCE_TYPE>(resource_mutator_step->m_type), resource_mutator_step->m_quantity);
    bool found = false;
    // If resource already on the tile increment its quantity.
    for (auto& r : tile->m_resources) {
      if (r.m_type == new_resource.m_type) {
        r.m_quantity += new_resource.m_quantity;
        found = true;
      }
    }
    if (!found) {
      tile->m_resources.push_back(new_resource);
    }
  }

  void execute_kill() {
    KillStep* kill_step = static_cast<KillStep*>(s_current_step);
    units::destroy(kill_step->m_unit_id);
    std::cout << kill_step->m_unit_id << " (id) has been slain." << std::endl;
  }

  std::string execute_pillage() {
    PillageStep* pillage_step = static_cast<PillageStep*>(s_current_step);
    Player* player = player::get_player(pillage_step->m_player);
    if (!player) return "Invalid player";
    Unit* unit = units::get_unit(pillage_step->m_unit);
    if (!unit) return "Invalid unit";
    Tile* tile = world_map::get_tile(unit->m_location);
    if (!tile) return "Invalid tile";
    for (size_t i = 0; i < tile->m_improvement_ids.size(); ++i) {
      uint32_t impId = tile->m_improvement_ids[i];
      Improvement* improvement = improvement::get_improvement(impId);
      if (!improvement) continue;
      if (improvement->m_owner_id == unit->m_owner_id) continue;
      std::cout << get_improvement_name(improvement->m_type) << " (owner " << improvement->m_owner_id << ") was pillaged by unit " << unit->m_unique_id << "!" << std::endl;
      improvement::destroy(improvement->m_unique_id);
      units::heal(unit->m_unique_id, 6.f);
      unit->m_action_points = 0;
    }
    return "";
  }

  std::string execute_spawn() {
    SpawnStep* spawn_step = static_cast<SpawnStep*>(s_current_step);
    Player* player = player::get_player(spawn_step->m_player);
    if (!player) return "Invalid player";
    Tile* tile = world_map::get_tile(spawn_step->m_location);
    if (!tile) return "Invalid location";
    units::create(static_cast<UNIT_TYPE>(spawn_step->m_unit_type), spawn_step->m_location, spawn_step->m_player);
    return "Unit created";
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

    // Run pathfinding to location
    std::vector<sf::Vector3i> path = search::path_to(unit->m_location, move_step->m_destination, world_map::get_map());
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
    while (step_move(units_to_move, unit->m_owner_id)) {

    }

    // Queue it for continued movement, unit will remove itself if it is done moving
    s_units_to_move.push_back(unit->m_unique_id);
  }

  std::string execute_purchase() {
    PurchaseStep* purchase_step = static_cast<PurchaseStep*>(s_current_step);
    Player* player = player::get_player(purchase_step->m_player);
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(purchase_step->m_city);
    if(!city) return "Invalid City";
    if (purchase_step->m_production_id != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(purchase_step->m_production_id));
      float cost = production::required_to_purchase(t);
      if (player->m_gold < cost) {
        ss << "Player has " << player->m_gold << " and needs " << cost << " gold. Purchase failed.";
        return ss.str();
      }
      player->m_gold -= cost;
      city->Purchase(t);
      return "Purchase made.";
    }

    std::vector<CONSTRUCTION_TYPE> available = city->GetConstruction()->Incomplete();
    ss << "City (" << city->m_id << ") available purchases: " << std::endl;
    for (size_t i = 0; i < available.size(); ++i) {
      CONSTRUCTION_TYPE t(available[i]);
      ss << production::required_to_purchase(t) << " Gold: " << get_construction_name(t) << std::endl;
    }
    return ss.str();
  }

  std::string execute_research() {
    ResearchStep* research_step = static_cast<ResearchStep*>(s_current_step);
    Player* player = player::get_player(research_step->m_player);
    if (!player) return "Invalid player";
    std::vector<uint32_t>& research = player->m_available_research;
    std::vector<uint32_t>::const_iterator findIt = find(research.begin(), research.end(), research_step->m_science);
    if (findIt == research.end()) return "Research is not available to player.";
    player->m_research = static_cast<SCIENCE_TYPE>(research_step->m_science);
    return "Research assigned.";
  }

  std::string execute_sell() {
    SellStep* sell_step = static_cast<SellStep*>(s_current_step);
    Player* player = player::get_player(sell_step->m_player);
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(sell_step->m_city);
    if(!city) return "Invalid City";
    std::vector<CONSTRUCTION_TYPE> completed = city->GetConstruction()->Complete();
    if (sell_step->m_production_id != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(sell_step->m_production_id));
      for (size_t i = 0; i < completed.size(); ++i) {
        if (completed[i] == t) {
          player->m_gold += production::yield_from_sale(completed[i]);
          city->GetConstruction()->Sell(t);
          return "Sale made.";
        }
      }
      ss << "Type not found." << std::endl;
    }

    ss << "City (" << city->m_id << ") available buildings for sale: " << std::endl;
    for (size_t i = 0; i < completed.size(); ++i) {
      CONSTRUCTION_TYPE t(completed[i]);
      ss << "+" << production::yield_from_sale(t) << " Gold: " << get_construction_name(t) << std::endl;
    }
    return ss.str();
  }

  std::string execute_siege() {
    SiegeStep* siege_step = static_cast<SiegeStep*>(s_current_step);
    Player* player = player::get_player(siege_step->m_player);
    if(!player) return "Invalid Player";
    City* city = city::get_city(siege_step->m_city);
    if(!city) return "Invalid City";
    Unit* unit = units::get_unit(siege_step->m_unit);
    if (!unit) return "Invalid Unit";
    if (!player->OwnsUnit(unit->m_unique_id)) return "Player doesn't own unit";
    Tile* tile = world_map::get_tile(city->m_location);
    if (!tile) return "Invalid City Location";
    if (tile->m_unit_ids.size()) return "City may not be sieged while units defend";
    // TODO: Should units have siege damage?
    city->Siege(unit->m_combat_stats.m_attack);
    return "Siege Occured";
  }

  void execute_magic() {
    MagicStep* magic_step = static_cast<MagicStep*>(s_current_step);
    magic::cast(magic_step->m_player, magic_step->m_type, magic_step->m_location, magic_step->m_cheat);
  }

  void execute_status() {
    StatusStep* status_step = static_cast<StatusStep*>(s_current_step);
    status_effect::create(status_step->m_type, status_step->m_location);
  }

  void execute_queue_move() {
    Unit* unit = generate_path();
    if(!unit) return;
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

  std::string execute_abort() {
    AbortStep* abort_step = static_cast<AbortStep*>(s_current_step);
    Player* player = player::get_player(abort_step->m_player);
    if (!player) return "Invalid Player";
    City* city = city::get_city(abort_step->m_city);
    if (!city) return "Invalid City";
    if (!player->OwnsCity(city->m_id)) return "Player doesn't own city.";
    city->GetConstruction()->Abort(abort_step->m_index);
    return "Removing...";
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

  std::string execute_city_defense() {
    CityDefenseStep* city_defense_step = static_cast<CityDefenseStep*>(s_current_step);
    Player* player = player::get_player(city_defense_step->m_player);
    if(!player) return "Invalid Player";
    Unit* unit = units::get_unit(city_defense_step->m_unit);
    if(!unit) return "Invalid Unit";
    uint32_t cityId = 0;
    auto find_defending_city = [player, &cityId](const City& city) {
      if (!player->OwnsCity(city.m_id)) return false;
      if (city.m_defenses_used) return false;
      cityId = city.m_id;
      return true;
    };
    search::bfs_cities(unit->m_location, 2, world_map::get_map(), find_defending_city);

    City* city = city::get_city(cityId);
    if(!city) return "No valid city found";
    city->m_defenses_used = true;
    std::cout << "Keeeerrrthunk. The city " << cityId << " has attacked unit " << unit->m_unique_id << std::endl;
    units::damage(unit->m_unique_id, 4.f);
    
    return "";
  }
}

void simulation::start() {
  // Magic numbers
  sf::Vector3i start;
  world_map::build(start, 10);
  // Setup unit definitions
  unit_definitions::initialize();
  science::initialize();
  magic::initialize();
  world_map::load_file("marin.dat");
}

void simulation::kill() {
  units::clear();
  city::clear();
  science::shutdown();
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
  size_t unit_count = 0;
  player::for_each_player_unit(player_id, [&unit_count] (Unit& u) {
    if (u.m_path.empty()) {
      ++unit_count;
    }
  });
  if (unit_count) {
    std::cout << "Player has " << unit_count << " idle_unit." << std::endl;
  }
  simulation::science_notifications();
}

void simulation::process_step(Step* step) {
  // Save a pointer to the current step
  s_current_step = step;

  // Process the step
  switch (step->m_command) {
    case COMMAND_TYPE::QUIT:
      break;
    case COMMAND_TYPE::BEGIN_TURN:
      process_begin_turn();
      break;
    case COMMAND_TYPE::END_TURN:
      process_end_turn();
      break;
    case COMMAND_TYPE::ATTACK:
      execute_attack();
      break;
    case COMMAND_TYPE::ABORT:
      std::cout << execute_abort() << std::endl;
      break;
    case COMMAND_TYPE::COLONIZE:
      execute_colonize();
      break;
    case COMMAND_TYPE::CONSTRUCT:
      execute_construction();
      break;
    case COMMAND_TYPE::DISCOVER:
      break;
    case COMMAND_TYPE::GRANT:
      execute_grant();
      break;
    case COMMAND_TYPE::HARVEST:
      execute_harvest();
      break;
    case COMMAND_TYPE::IMPROVE:
      execute_improve();
      break;
    case COMMAND_TYPE::TILE_MUTATOR:
      execute_tile_mutator();
      break;
    case COMMAND_TYPE::RESOURCE_MUTATOR:
      execute_resource_mutator();
      break;
    case COMMAND_TYPE::KILL:
      execute_kill();
      break;
    case COMMAND_TYPE::MOVE:
      execute_move();
      break;
    case COMMAND_TYPE::PILLAGE:
      std::cout << execute_pillage() << std::endl;;
      break;
    case COMMAND_TYPE::QUEUE_MOVE:
      execute_queue_move();
      break;
    case COMMAND_TYPE::PURCHASE:
      std::cout << execute_purchase() << std::endl;
      break;
    case COMMAND_TYPE::RESEARCH:
      std::cout << execute_research() << std::endl;
      break;
    case COMMAND_TYPE::SPECIALIZE:
      execute_specialize();
      break;
    case COMMAND_TYPE::SELL:
      std::cout << execute_sell() << std::endl;
      break;
    case COMMAND_TYPE::SIEGE:
      std::cout << execute_siege() << std::endl;
      break;
    case COMMAND_TYPE::SPAWN:
      std::cout << execute_spawn() << std::endl;
      break;
    case COMMAND_TYPE::ADD_PLAYER:
      execute_add_player();
      return; // Special case, adding a player does not have output
    case COMMAND_TYPE::MODIFY_UNIT_STATS:
      execute_modify_stats();
      return; // Modifying stats also does not have output
    case COMMAND_TYPE::BARBARIAN_TURN:
      // Process the barbarian turn.
      // FIXME
      return;
    case COMMAND_TYPE::CITY_DEFENSE:
      std::cout << execute_city_defense() << std::endl;
      return;
    case COMMAND_TYPE::MAGIC:
      execute_magic();
      break;
    case COMMAND_TYPE::STATUS:
      execute_status();
      break;
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

void simulation::process_begin_turn() {

  if (!player::all_players_turn_ended()) {
    std::cout << "Not all players ready for next turn. " << std::endl;
    return;
  }

  // Apply changes
  phase_city_growth();
  phase_science_progression();
  phase_diplomatic_progression();
  phase_spawn_units();
  phase_spawn_buildings();

  // Provide turn feedback
  phase_global_events();

  // Increment turn counter
  ++s_current_turn;

  // Each player state -> Playing
  player::for_each_player([](Player& player) {
    player.m_turn_state = TURN_TYPE::TURNACTIVE;
  });
  std::cout << std::endl << "Beginning turn #" << s_current_turn << std::endl;

  phase_restore_actions();
  status_effect::process();
}

void simulation::process_end_turn() {
  EndTurnStep* end_step = static_cast<EndTurnStep*>(s_current_step);

  Player* player = player::get_player(end_step->m_player);
  if (!player) return;

  if (player->m_ai_type == AI_TYPE::BARBARIAN) {
    barbarians::pillage_and_plunder(end_step->m_player);
  }

  phase_queued_movement(end_step->m_player); //TODO: pass player filter to movement
  player->m_turn_state = TURN_TYPE::TURNCOMPLETED;
  if (player::all_players_turn_ended()) {
    process_begin_turn(); 
  }

  player_notifications(end_step->m_next_player);
}
