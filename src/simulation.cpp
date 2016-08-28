#include "simulation.h"

#include "search.h"
#include "unit.h"
#include "city.h"
#include "format.h"
#include "world_map.h"
#include "hex.h"
#include "util.h"
#include "player.h"
#include "production.h"
#include "unit_definitions.h"
#include "unique_id.h"
#include "improvement.h"
#include "game_types.h"
#include "terrain_yield.h"
#include "ai_barbarians.h"
#include "ai_monster.h"
#include "science.h"
#include "magic.h"
#include "custom_math.h"
#include "status_effect.h"
#include "notification.h"
#include "network_types.h"
#include "scenario.h"

#include <iostream>
#include <algorithm>

namespace simulation {
  // Units to move in the current step
  typedef std::vector<uint32_t> UnitMovementVector;
  UnitMovementVector s_units_to_move;
  // Units to fight in the current step
  std::vector<std::pair<uint32_t, uint32_t> > s_units_to_fight;

  // Used to count turns and index into player array
  uint32_t s_current_turn = 0;

  // Order of operations that should be checked after a step
  bool step_move(UnitMovementVector& units_to_move, uint32_t player_id);

  void step_combat();

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
      Unit* unit = unit::get_unit(unit_id);
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
              Unit* other = unit::get_unit(id);
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

  void update_combat() {
    for (auto pair : s_units_to_fight) {
      Unit* unit = unit::get_unit(pair.first);
      if (!unit) {
        continue;
      }

      Unit* defender = unit::get_unit(pair.second);
      if (!defender) {
        continue;
      }

      if (!unit->m_action_points) {
        std::cout << "Unit " << unit->m_id << " (id) is too exhausted to initiate combat. " << std::endl;
        continue;
      }

      // Attacker faces defender on combat initiation.
      unit::change_direction(pair.first, defender->m_location);

      // If combat occurs deplete action points from the initiator
      if (unit::combat(pair.first, pair.second)) {
        unit->m_action_points = 0;
      }

      // Defender turns to face attacker after combat.
      unit::change_direction(pair.second, unit->m_location);
    }

    // Attacks should all complete in a single step?
    s_units_to_fight.clear();
  }

  void phase_city_growth() {
    std::vector<uint32_t> razed;
    auto city_func = [&razed](City& cityInstance, Player& player) { 
      TerrainYield t = cityInstance.DumpYields();
      std::cout << t << std::endl;
      cityInstance.Simulate(t);
      player.m_gold += t.m_gold;
      player.m_science += t.m_science;
      player.m_magic += t.m_magic;

      if (cityInstance.GetPopulation() <= 0.0) {
        razed.push_back(cityInstance.m_id);
      }
    };
    auto player_func = [city_func](Player& player) {
      player::for_each_player_city(player.m_id,
        [city_func, &player](City& c) { city_func(c, player); });
    };
    // Important: for_each_player is an ordered std::set
    // We must process players in order for consistent simulation results
    player::for_each_player(player_func);

    for (auto r : razed) {
      city::raze(r);
    }
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
    unit::replenish_actions();
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

  void execute_construction(const void* buffer, size_t buffer_len) {
    ConstructionStep construction_step;
    deserialize(buffer, buffer_len, construction_step);
    Player* player = player::get_player(construction_step.get_player());
    if (!player) {
      std::cout << "Invalid player index" << std::endl;
      return;
    }
    City* city = city::get_city(construction_step.get_city_id());
    if(!city) {
      std::cout << "City does not exist" << std::endl;
      return;
    }
    if (!player->OwnsCity(construction_step.get_city_id())) {
      std::cout << "Player does not own city" << std::endl;
      return;
    }
    CONSTRUCTION_TYPE t(production::id(construction_step.get_production_id()));

    if (construction_step.get_cheat()) {
      production_queue::purchase(city->GetProductionQueue(), t);
      return;
    }

    production_queue::add(city->GetProductionQueue(), t);
  }

  void execute_colonize(const void* buffer, size_t buffer_len) {
    ColonizeStep colonize_step;
    deserialize(buffer, buffer_len, colonize_step);

    Player* player = player::get_player(colonize_step.get_player());
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    bool too_close = false;
    sf::Vector3i location = colonize_step.get_location();
    search::bfs(location, 3, world_map::get_map(),
      [&too_close](const Tile& tile) {
      if (tile.m_city_id != unique_id::INVALID_ID) {
        std::cout << "Colonization failed: City (" << tile.m_city_id << ") is too close." << std::endl;
        too_close = true;
        return true;
      }
      return false;
    });
    if (too_close) return;

    uint32_t id = city::create(BUILDING_TYPE::TOWN, location, colonize_step.get_player());
    if (!id) {
      // Colonization failed.
      return;
    }
    // If colonization succeeded there is a worker on the tile, destroy it.
    player::add_city(colonize_step.get_player(), id);
    std::cout << "player " << player->m_name << " colonized city (" << id << ") at: " << format::vector3(location) << std::endl;
    Tile* t = world_map::get_tile(location);
    for (auto uid : t->m_unit_ids) {
      Unit* u = unit::get_unit(uid);
      if (!u) continue;
      // Consume the unit that built the city.
      if (u->m_type == UNIT_TYPE::WORKER && u->m_owner_id == player->m_id) {
        unit::destroy(u->m_id);
        break;
      }
    }
  }

  void execute_improve(const void* buffer, size_t buffer_len) {
    ImproveStep improve_step;
    deserialize(buffer, buffer_len, improve_step);

    Player* player = player::get_player(improve_step.get_player());
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    sf::Vector3i location = improve_step.get_location();
    Tile* tile = world_map::get_tile(location);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    size_t i = 0;
    Unit* unit = nullptr;
    uint32_t unit_id = 0;
    for (; i < tile->m_unit_ids.size(); ++i) {
      unit_id = tile->m_unit_ids[i];
      unit = unit::get_unit(unit_id);
      if (!unit) continue;
      if (!unit->m_action_points) continue;
      if (unit->m_type != UNIT_TYPE::WORKER) continue;
      break;
    }

    if (!unit || unit->m_type != UNIT_TYPE::WORKER) {
      std::cout << "No worker is available to improve the tile" << std::endl;
      return;
    }

    RESOURCE_TYPE rt = static_cast<RESOURCE_TYPE>(improve_step.get_resource());
    i = 0;
    for (; i < tile->m_resources.size(); ++i) {
      if (tile->m_resources[i].m_type == rt) break;
    }
    if (i == tile->m_resources.size()) {
      std::cout << "Resource not available on this tile." << std::endl;
      return;
    }
    Resource& res = tile->m_resources[i];
    IMPROVEMENT_TYPE impv = improvement::resource_improvement(res.m_type);
    if (!improvement::satisfies_requirements(res.m_type, impv, location)) {
      return;
    }

    uint32_t pid = improve_step.get_player();

    auto end_turn_inject = [res, impv, pid, location, unit_id]() {
      uint32_t id = improvement::create(res, impv, location, pid);

      if (id) {
        Unit* u = unit::get_unit(unit_id);
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
    if (status_effect::create(STATUS_TYPE::CONSTRUCTING_IMPROVEMENT, location)) {
      // Drain workers action points.
      unit->m_action_points = 0;
    }
  }

  void execute_grant(const void* buffer, size_t buffer_len) {
    GrantStep grant_step;
    deserialize(buffer, buffer_len, grant_step);
    SCIENCE_TYPE st = static_cast<SCIENCE_TYPE>(grant_step.get_science());
    science::research_complete(grant_step.get_player(), science::Science(st));
  }

  void execute_harvest(const void* buffer, size_t buffer_len) {
    HarvestStep harvest_step;
    deserialize(buffer, buffer_len, harvest_step);

    Player* player = player::get_player(harvest_step.get_player());
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    sf::Vector3i destination = harvest_step.get_destination();
    Tile* tile = world_map::get_tile(destination);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    if (terrain_yield::is_harvested(destination)) {
      terrain_yield::remove_harvest(destination);
      return;
    }
    City* city = city::nearest_city(destination);
    if (!city) {
      std::cout << "No valid city found" << std::endl;
      return; 
    }
    if (!player->OwnsCity(city->m_id)) {
      std::cout << "Terrain is not owned by this player" << std::endl;
      return;
    }
    if (terrain_yield::add_harvest(destination, city)) {
      std::cout << "City (" << city->m_id << ") is now harvesting from " << get_terrain_name(tile->m_terrain_type) << "." << std::endl;
      return;
    }
}

  void execute_tile_mutator(const void* buffer, size_t buffer_len) {
    TileMutatorStep tile_mutator_step;
    deserialize(buffer, buffer_len, tile_mutator_step);

    Tile* tile = world_map::get_tile(tile_mutator_step.get_destination());
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    tile->m_path_cost = tile_mutator_step.get_movement_cost();
  }

  void execute_specialize(const void* buffer, size_t buffer_len) {
    SpecializeStep specialize_step;
    deserialize(buffer, buffer_len, specialize_step);

    City* city = city::get_city(specialize_step.get_city_id());
    if (!city) {
      std::cout << "Invalid city" << std::endl;
      return;
    }
    Player* player = player::get_player(specialize_step.get_player());
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
    if (city->SetSpecialization(static_cast<TERRAIN_TYPE>(specialize_step.get_terrain_type()))) {
      std::cout << "City has specialized in " << get_terrain_name(static_cast<TERRAIN_TYPE>(specialize_step.get_terrain_type())) << std::endl;
    }
  }

  void execute_resource_mutator(const void* buffer, size_t buffer_len) {
    ResourceMutatorStep resource_mutator_step;
    deserialize(buffer, buffer_len, resource_mutator_step);

    Tile* tile = world_map::get_tile(resource_mutator_step.get_destination());
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    Resource new_resource(static_cast<RESOURCE_TYPE>(resource_mutator_step.get_type()), resource_mutator_step.get_quantity());
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

  void execute_kill(const void* buffer, size_t buffer_len) {
    KillStep kill_step;
    deserialize(buffer, buffer_len, kill_step);

    unit::destroy(kill_step.get_unit_id());
    std::cout << kill_step.get_unit_id() << " (id) has been slain." << std::endl;
  }

  std::string execute_pillage(const void* buffer, size_t buffer_len) {
    PillageStep pillage_step;
    deserialize(buffer, buffer_len, pillage_step);

    Player* player = player::get_player(pillage_step.get_player());
    if (!player) return "Invalid player";
    Unit* unit = unit::get_unit(pillage_step.get_unit());
    if (!unit) return "Invalid unit";
    Tile* tile = world_map::get_tile(unit->m_location);
    if (!tile) return "Invalid tile";
    for (size_t i = 0; i < tile->m_improvement_ids.size(); ++i) {
      uint32_t impId = tile->m_improvement_ids[i];
      Improvement* improvement = improvement::get_improvement(impId);
      if (!improvement) continue;
      if (improvement->m_owner_id == unit->m_owner_id) continue;
      std::cout << get_improvement_name(improvement->m_type) << " (owner " << improvement->m_owner_id << ") was pillaged by unit " << unit->m_id << "!" << std::endl;
      improvement::destroy(improvement->m_id);
      unit::heal(unit->m_id, 6.f);
      unit->m_action_points = 0;
    }
    return "";
  }

  std::string execute_spawn(const void* buffer, size_t buffer_len) {
    SpawnStep spawn_step;
    deserialize(buffer, buffer_len, spawn_step);

    Player* player = player::get_player(spawn_step.get_player());
    if (!player) return "Invalid player";
    Tile* tile = world_map::get_tile(spawn_step.get_location());
    if (!tile) return "Invalid location";
    unit::create(static_cast<UNIT_TYPE>(spawn_step.get_unit_type()), spawn_step.get_location(), spawn_step.get_player());
    return "Unit created";
  }

  Unit* generate_path(MoveStep& move_step) {
    bool avoid_city = move_step.get_avoid_city();
    bool avoid_unit = move_step.get_avoid_unit();
    sf::Vector3i destination = move_step.get_destination();
    uint32_t unitId = move_step.get_unit_id();

    auto find_tiles = [avoid_city, avoid_unit](const Tile& tile) -> bool {
      if (avoid_city && tile.m_city_id) return false;
      if (avoid_unit && !tile.m_unit_ids.empty()) return false;
      return true;
    };
    Player* player = player::get_player(move_step.get_player());
    Unit* unit = unit::get_unit(unitId);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return nullptr;
    }
    if (!unit) {
      std::cout << "Unit: " << unitId << " does not exist." << std::endl;
      return nullptr;
    }
    if (player && !player->OwnsUnit(unitId)) {
      std::cout << "Player does not own unit" << std::endl;
      return nullptr;
    }
    Tile* destination_tile = world_map::get_tile(destination);
    if (destination_tile == nullptr) {
      std::cout << "Invalid destination: " << format::vector3(destination) << std::endl;
      return nullptr;
    }

    // Run pathfinding to location
    std::vector<sf::Vector3i> path = search::path_to(unit->m_location, destination, world_map::get_map(), find_tiles);
    if (!path.empty()) {
      path.erase(path.begin());
      if (avoid_city && destination_tile->m_city_id) {
        path.pop_back();
      }
      else if (avoid_unit && !destination_tile->m_unit_ids.empty()) {
        path.pop_back();
      }
    }
    // Set path
    unit::set_path(unit->m_id, path);
    return unit;
  }


  void execute_setpath() {
    //SetPathStep* path_step = static_cast<SetPathStep*>(s_current_step);
    //Player* player = player::get_player(path_step->m_player);
    //Unit* unit = units::get_unit(path_step->m_unit_id);
    //if (!player) {
    //  std::cout << "Invalid player" << std::endl;
    //  return;
    //}
    //if (!unit) {
    //  std::cout << "Unit: " << path_step->m_unit_id << " does not exist." << std::endl;
    //  return;
    //}
    //if (player && !player->OwnsUnit(path_step->m_unit_id)) {
    //  std::cout << "Player does not own unit" << std::endl;
    //  return;
    //}
    //units::set_path(path_step->m_unit_id, path_step->m_path);
    //UnitMovementVector units_to_move;
    //units_to_move.push_back(unit->m_unique_id);
    //while (step_move(units_to_move, unit->m_owner_id));

    //// Queue it for continued movement, unit will remove itself if it is done moving
    //s_units_to_move.push_back(unit->m_unique_id);
  }

  void execute_move(const void* buffer, size_t buffer_len) {
    std::cout << "Executing immediate movement " << std::endl;
    // Just set where the unit needs to move and add it to a list. The actual move will happen in the move phase
    MoveStep move_step;
    deserialize(buffer, buffer_len, move_step);

    Unit* unit = generate_path(move_step);
    if (!unit) return;

    if (move_step.get_immediate()) {
      UnitMovementVector units_to_move;
      units_to_move.push_back(unit->m_id);
      while (step_move(units_to_move, unit->m_owner_id)) {
      }
    }
    // Queue it for continued movement, unit will remove itself if it is done moving
    s_units_to_move.push_back(unit->m_id);
  }

  std::string execute_purchase(const void* buffer, size_t buffer_len) {
    PurchaseStep purchase_step;
    deserialize(buffer, buffer_len, purchase_step);

    Player* player = player::get_player(purchase_step.get_player());
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(purchase_step.get_city());
    if(!city) return "Invalid City";
    if (purchase_step.get_production() != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(purchase_step.get_production()));
      float cost = production::required_to_purchase(t);
      if (player->m_gold < cost) {
        ss << "Player has " << player->m_gold << " and needs " << cost << " gold. Purchase failed.";
        return ss.str();
      }
      player->m_gold -= cost;
      production_queue::purchase(city->GetProductionQueue(), t);
      return "Purchase made.";
    }

    std::vector<CONSTRUCTION_TYPE> available = production_queue::incomplete(city->GetProductionQueue());
    ss << "City (" << city->m_id << ") available purchases: " << std::endl;
    for (size_t i = 0; i < available.size(); ++i) {
      CONSTRUCTION_TYPE t(available[i]);
      ss << production::required_to_purchase(t) << " Gold: " << get_construction_name(t) << std::endl;
    }
    return ss.str();
  }

  std::string execute_research(const void* buffer, size_t buffer_len) {
    ResearchStep research_step;
    deserialize(buffer, buffer_len, research_step);

    Player* player = player::get_player(research_step.get_player());
    if (!player) return "Invalid player";
    std::vector<uint32_t>& research = player->m_available_research;
    std::vector<uint32_t>::const_iterator findIt = find(research.begin(), research.end(), research_step.get_science());
    if (findIt == research.end()) return "Research is not available to player.";
    player->m_research = static_cast<SCIENCE_TYPE>(research_step.get_science());
    return "Research assigned.";
  }

  std::string execute_sell(const void* buffer, size_t buffer_len) {
    SellStep sell_step;
    deserialize(buffer, buffer_len, sell_step);

    Player* player = player::get_player(sell_step.get_player());
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(sell_step.get_city());
    if(!city) return "Invalid City";
    std::vector<CONSTRUCTION_TYPE> completed = production_queue::complete(city->GetProductionQueue());
    if (sell_step.get_production_id() != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(sell_step.get_production_id()));
      for (size_t i = 0; i < completed.size(); ++i) {
        if (completed[i] == t) {
          player->m_gold += production::yield_from_sale(completed[i]);
          production_queue::sell(city->GetProductionQueue(), t);
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

  std::string execute_siege(const void* buffer, size_t buffer_len) {
    SiegeStep siege_step;
    deserialize(buffer, buffer_len, siege_step);

    Player* player = player::get_player(siege_step.get_player());
    if(!player) return "Invalid Player";
    City* city = city::get_city(siege_step.get_city());
    if(!city) return "Invalid City";
    Unit* unit = unit::get_unit(siege_step.get_unit());
    if (!unit) return "Invalid Unit";
    if (!player->OwnsUnit(unit->m_id)) return "Player doesn't own unit";
    Tile* tile = world_map::get_tile(city->m_location);
    if (!tile) return "Invalid City Location";
    if (unit->m_action_points == 0) return "Unit is exhausted.";
    if (tile->m_unit_ids.size()) return "City may not be sieged while units defend";
    if (unit->m_combat_stats.m_range < hex::cube_distance(unit->m_location, city->m_location)) return "Unit doesn't have sufficient range to siege from here";
    // TODO: Should units have siege damage?
    city->Siege(unit->m_combat_stats.m_attack);
    unit->m_action_points = 0;
    return "Siege Occured";
  }

  void execute_magic(const void* buffer, size_t buffer_len) {
    MagicStep magic_step;
    deserialize(buffer, buffer_len, magic_step);
    
    magic::cast(magic_step.get_player(), magic_step.get_type(), magic_step.get_location(), magic_step.get_cheat());
  }

  void execute_status(const void* buffer, size_t buffer_len) {
    StatusStep status_step;
    deserialize(buffer, buffer_len, status_step);
    status_effect::create(status_step.get_type(), status_step.get_location());
  }

  void execute_scenario(const void* buffer, size_t buffer_len) {
    ScenarioStep scenario_step;
    deserialize(buffer, buffer_len, scenario_step);
    scenario::start(scenario_step.get_type());
  }

  void execute_add_player(const void* buffer, size_t buffer_len) {
    AddPlayerStep player_step;
    deserialize(buffer, buffer_len, player_step);

    switch (player_step.get_ai_type()) {
      case AI_TYPE::BARBARIAN:
        barbarians::set_player_id(player::create_ai(player_step.get_ai_type()));
        break;
      case AI_TYPE::HUMAN:
        player::create_human(player_step.get_name());
      case AI_TYPE::UNKNOWN:
      default:
        break;
    }
  }

  void execute_attack(const void* buffer, size_t buffer_len) {
    // Add a fight to the list to be executed in the combat phase
    AttackStep attack_step;
    deserialize(buffer, buffer_len, attack_step);
    Player* player = player::get_player(attack_step.get_player());
    if (!player) {
      std::cout << "Invalid player for attack action" << std::endl;
      return;
    }
    if (!player->OwnsUnit(attack_step.get_attacker_id())) {
      std::cout << "Player does not own attacking unit" << std::endl;
      return;
    }
    s_units_to_fight.push_back(std::pair<uint32_t, uint32_t>(attack_step.get_attacker_id(), attack_step.get_defender_id()));
  }

  std::string execute_production_move(const void* buffer, size_t buffer_len) {
    ProductionMoveStep move_step;
    deserialize(buffer, buffer_len, move_step);

    Player* player = player::get_player(move_step.get_player());
    if (!player) return "Invalid Player";
    City* city = city::get_city(move_step.get_city());
    if (!city) return "Invalid City";
    if (!player->OwnsCity(city->m_id)) return "Player doesn't own city.";
    production_queue::move(city->GetProductionQueue(), move_step.get_source_index(), move_step.get_destination_index());
    return "Production move completed.";
  }

  std::string execute_production_abort(const void* buffer, size_t buffer_len) {
    ProductionAbortStep abort_step;
    deserialize(buffer, buffer_len, abort_step);
    
    Player* player = player::get_player(abort_step.get_player());
    if (!player) return "Invalid Player";
    City* city = city::get_city(abort_step.get_city());
    if (!city) return "Invalid City";
    if (!player->OwnsCity(city->m_id)) return "Player doesn't own city.";
    production_queue::remove(city->GetProductionQueue(), abort_step.get_index());
    return "Removing...";
  }

  void execute_modify_stats(const void* buffer, size_t buffer_len) {

    UnitStatsStep stats_step;
    deserialize(buffer, buffer_len, stats_step);

    Unit* unit = unit::get_unit(stats_step.get_unit_id());
    if (!unit) {
      return;
    }
    unit->m_combat_stats.m_health = static_cast<float>(stats_step.get_health());
    unit->m_combat_stats.m_attack = static_cast<float>(stats_step.get_attack());
    unit->m_combat_stats.m_range = static_cast<float>(stats_step.get_range());

    std::cout << format::combat_stats(unit->m_combat_stats) << std::endl;
  }

  std::string execute_city_defense(const void* buffer, size_t buffer_len) {
    CityDefenseStep city_defense_step;
    deserialize(buffer, buffer_len, city_defense_step);

    Player* player = player::get_player(city_defense_step.get_player());
    if(!player) return "Invalid Player";
    Unit* unit = unit::get_unit(city_defense_step.get_unit());
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
    std::cout << "Keeeerrrthunk. The city " << cityId << " has attacked unit " << unit->m_id << std::endl;
    unit::damage(unit->m_id, 4.f);
    
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
  unit::clear();
  city::clear();
  science::shutdown();
}

uint32_t simulation::get_turn() {
  return s_current_turn;
}

void simulation::process_step(const void* buffer, size_t buffer_len) {
  NETWORK_TYPE t = read_type(buffer, buffer_len);

  // Process the step
  switch (t) {
  case NETWORK_TYPE::QUITSTEP:
    break;
  case NETWORK_TYPE::BEGINSTEP:
    process_begin_turn();
    break;
  case NETWORK_TYPE::ENDTURNSTEP:
    process_end_turn(buffer, buffer_len);
    break;
  case NETWORK_TYPE::ATTACKSTEP:
    execute_attack(buffer, buffer_len);
    break;
  case NETWORK_TYPE::PRODUCTIONABORTSTEP:
    std::cout << execute_production_abort(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::PRODUCTIONMOVESTEP:
    std::cout << execute_production_move(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::COLONIZESTEP:
    execute_colonize(buffer, buffer_len);
    break;
  case NETWORK_TYPE::CONSTRUCTIONSTEP:
    execute_construction(buffer, buffer_len);
    break;
  case NETWORK_TYPE::GRANTSTEP:
    execute_grant(buffer, buffer_len);
    break;
  case NETWORK_TYPE::HARVESTSTEP:
    execute_harvest(buffer, buffer_len);
    break;
  case NETWORK_TYPE::IMPROVESTEP:
    execute_improve(buffer, buffer_len);
    break;
  case NETWORK_TYPE::TILEMUTATORSTEP:
    execute_tile_mutator(buffer, buffer_len);
    break;
  case NETWORK_TYPE::RESOURCEMUTATORSTEP:
    execute_resource_mutator(buffer, buffer_len);
    break;
  case NETWORK_TYPE::KILLSTEP:
    execute_kill(buffer, buffer_len);
    break;
  case NETWORK_TYPE::MOVESTEP:
    execute_move(buffer, buffer_len);
    break;
  case NETWORK_TYPE::PILLAGESTEP:
    std::cout << execute_pillage(buffer, buffer_len) << std::endl;;
    break;
  case NETWORK_TYPE::PURCHASESTEP:
    std::cout << execute_purchase(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::RESEARCHSTEP:
    std::cout << execute_research(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::SPECIALIZESTEP:
    execute_specialize(buffer, buffer_len);
    break;
  case NETWORK_TYPE::SELLSTEP:
    std::cout << execute_sell(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::SIEGESTEP:
    std::cout << execute_siege(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::SPAWNSTEP:
    std::cout << execute_spawn(buffer, buffer_len) << std::endl;
    break;
  case NETWORK_TYPE::ADDPLAYERSTEP:
    execute_add_player(buffer, buffer_len);
    return; // Special case, adding a player does not have output
  case NETWORK_TYPE::UNITSTATSSTEP:
    execute_modify_stats(buffer, buffer_len);
    return; // Modifying stats also does not have output
  case NETWORK_TYPE::BARBARIANSTEP:
    return;
  case NETWORK_TYPE::CITYDEFENSESTEP:
    std::cout << execute_city_defense(buffer, buffer_len) << std::endl;
    return;
  case NETWORK_TYPE::MAGICSTEP:
    execute_magic(buffer, buffer_len);
    break;
  case NETWORK_TYPE::STATUSSTEP:
    execute_status(buffer, buffer_len);
    break;
  case NETWORK_TYPE::SCENARIOSTEP:
    execute_scenario(buffer, buffer_len);
    break;
  default:
    break;
  }

  // Well, this is a weird one-off
  update_combat();
}

// Same as regular step, just deletes step on behalf of ai when finished.
void simulation::process_step_from_ai(const void* buffer, size_t buffer_len) {
  simulation::process_step(buffer, buffer_len);
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

  // Run scenarios
  scenario::process();
}

void simulation::process_end_turn(const void* buffer, size_t buffer_len) {
  EndTurnStep end_step;
  deserialize(buffer, buffer_len, end_step);

  Player* player = player::get_player(end_step.get_player());
  if (!player) return;

  if (player->m_ai_type == AI_TYPE::BARBARIAN) {
    barbarians::pillage_and_plunder(end_step.get_player());
  }

  if (player->m_ai_type == AI_TYPE::MONSTER) {
    monster::execute_turn(end_step.get_player());
  }

  phase_queued_movement(end_step.get_player()); 
  player->m_turn_state = TURN_TYPE::TURNCOMPLETED;
  if (player::all_players_turn_ended()) {
    process_begin_turn(); 
  }

  std::cout << std::endl << "Active player is " << end_step.get_next_player() << std::endl;
  NotificationVector events = notification::get_player_notifications(end_step.get_next_player());
  for (size_t i = 0; i < events.size(); ++i) {
    std::cout << notification::to_string(events[i]) << std::endl;
  }
}
