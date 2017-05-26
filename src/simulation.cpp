#include "simulation.h"

#include "ai_empire_trees.h"
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
#include "scenario_citylife.h"
#include "tile_costs.h"

#include "step_generated.h"

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

  sf::Vector3i get_v3i(const fbs::v3i* vec3i) {
    sf::Vector3i ret(vec3i->x(), vec3i->y(), vec3i->z());
    return ret;
  }

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

      // If combat occurs deplete action points from the initiator
      if (unit::combat(pair.first, pair.second)) {
        unit->m_action_points = 0;
      }
    }

    // Attacks should all complete in a single step?
    s_units_to_fight.clear();
  }

  void phase_city_raze() {
    std::vector<uint32_t> razed;
    auto city_func = [&razed](City& cityInstance, Player& player)  {
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

  void execute_construction(const fbs::ConstructionStep* construction_step) {
    uint32_t player_id = construction_step->player();
    uint32_t city_id = construction_step->city_id();
    uint32_t production_id = construction_step->production_id();
    bool cheat = construction_step->cheat();
    Player* player = player::get_player(player_id);
    if (!player) {
      std::cout << "Invalid player index" << std::endl;
      return;
    }
    City* city = city::get_city(city_id);
    if(!city) {
      std::cout << "City does not exist" << std::endl;
      return;
    }
    if (!player->OwnsCity(city_id)) {
      std::cout << "Player does not own city" << std::endl;
      return;
    }
    CONSTRUCTION_TYPE t(production::id(production_id));

    if (cheat) {
      production_queue::purchase(city->GetProductionQueue(), t);
      return;
    }

    production_queue::add(city->GetProductionQueue(), t);
  }

  void execute_colonize(const fbs::ColonizeStep* colonize_step) {
    uint32_t player_id = colonize_step->player();
    sf::Vector3i location = get_v3i(colonize_step->location());
    Player* player = player::get_player(player_id);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
    bool too_close = false;
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

    uint32_t id = city::create(BUILDING_TYPE::TOWN, location, player_id);
    if (!id) {
      // Colonization failed.
      return;
    }
    // If colonization succeeded there is a worker on the tile, destroy it.
    player::add_city(player_id, id);
    std::cout << "player " << player->m_name << " colonized city (" << id << ") at: " << format::vector3(location) << std::endl;
    Tile* t = world_map::get_tile(location);
    for (auto uid : t->m_unit_ids) {
      Unit* u = unit::get_unit(uid);
      if (!u) continue;
      // Consume the unit that built the city.
      if (u->m_type == UNIT_TYPE::WORKER && u->m_owner_id == player->m_id) {
        unit::destroy(u->m_id, unique_id::INVALID_ID, unique_id::INVALID_PLAYER);
        break;
      }
    }
  }

  void execute_improve(const fbs::ImproveStep* improve_step) {
    uint32_t player_id = improve_step->player();
    sf::Vector3i location = get_v3i(improve_step->location());
    Player* player = player::get_player(player_id);
    uint32_t resource_id = improve_step->resource();
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
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

    RESOURCE_TYPE rt = static_cast<RESOURCE_TYPE>(resource_id);
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

    auto end_turn_inject = [res, impv, player_id, location, unit_id]() {
      uint32_t id = improvement::create(res, impv, location, player_id);

      if (id) {
        Unit* u = unit::get_unit(unit_id);
        if (!u) { 
          std::cout << "unit no longer exists." << std::endl;
          return;
        }
        std::cout << "adding improvement to player: " << player_id << std::endl;
        player::add_improvement(player_id, id);
        u->m_action_points = 0;
      } 
    };

    status_effect::inject_end(end_turn_inject);
    if (status_effect::create(STATUS_TYPE::CONSTRUCTING_IMPROVEMENT, location)) {
      // Drain workers action points.
      unit->m_action_points = 0;
    }
  }

  void execute_grant(const fbs::GrantStep* grant_step) {
    SCIENCE_TYPE st = static_cast<SCIENCE_TYPE>(grant_step->science());
    science::research_complete(grant_step->player(), science::Science(st));
  }

  void execute_harvest(const fbs::HarvestStep* harvest_step) {
    uint32_t player_id = harvest_step->player();
    sf::Vector3i destination = get_v3i(harvest_step->destination());
    Player* player = player::get_player(player_id);
    if (!player) {
      std::cout << "Invalid player" << std::endl;
      return;
    }
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

  void execute_tile_mutator(const fbs::TileMutatorStep* mutator_step) {
    sf::Vector3i destination = get_v3i(mutator_step->destination());
    uint32_t movement_cost = mutator_step->movement_cost();
    Tile* tile = world_map::get_tile(destination);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    tile->m_path_cost = movement_cost;
  }

  void execute_specialize(const fbs::SpecializeStep* specialize_step) {
    uint32_t city_id = specialize_step->city_id();
    uint32_t player_id = specialize_step->player();
    uint32_t terrain_type = specialize_step->terrain_type();
    City* city = city::get_city(city_id);
    if (!city) {
      std::cout << "Invalid city" << std::endl;
      return;
    }
    Player* player = player::get_player(player_id);
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
    if (city->SetSpecialization(static_cast<TERRAIN_TYPE>(terrain_type))) {
      std::cout << "City has specialized in " << get_terrain_name(static_cast<TERRAIN_TYPE>(terrain_type)) << std::endl;
    }
  }

  void execute_resource_mutator(const fbs::ResourceMutatorStep* rm_step) {
    sf::Vector3i dest = get_v3i(rm_step->destination());
    uint32_t resource_type = rm_step->type();
    uint32_t quantity = rm_step->quantity();
    Tile* tile = world_map::get_tile(dest);
    if (!tile) {
      std::cout << "Invalid tile" << std::endl;
      return;
    }
    Resource new_resource(static_cast<RESOURCE_TYPE>(resource_type), quantity);
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

  void execute_kill(const fbs::KillStep* kill_step) {
    uint32_t unit_id = kill_step->unit_id();
    unit::destroy(unit_id, unique_id::INVALID_ID, unique_id::INVALID_PLAYER);
    std::cout << unit_id << " (id) has been slain." << std::endl;
  }

  void reset() {
    s_units_to_move.clear();
    s_units_to_fight.clear();
    s_current_turn = 0;

    player::reset();
    unit::reset();
    city::reset();
    magic::reset();
    status_effect::reset();
    world_map::reset();
    production::reset();
    science::reset();
    barbarians::reset();
    improvement::reset();
    terrain_yield::reset();
    tile_costs::reset();
    unique_id::reset();
    unit_definitions::reset();
    scenario::reset();

    // Restart the simulation after everything is reset
    simulation::start();
  }

  std::string execute_pillage(const fbs::PillageStep* pillage_step) {
    uint32_t player_id = pillage_step->player();
    uint32_t unit_id = pillage_step->unit();
    Player* player = player::get_player(player_id);
    if (!player) return "Invalid player";
    Unit* unit = unit::get_unit(unit_id);
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

  std::string execute_spawn(const fbs::SpawnStep* spawn_step) {
    uint32_t player_id = spawn_step->player();
    sf::Vector3i location = get_v3i(spawn_step->location());
    uint32_t unit_type = spawn_step->unit_type();
    Player* player = player::get_player(player_id);
    if (!player) return "Invalid player";
    Tile* tile = world_map::get_tile(location);
    if (!tile) return "Invalid location";
    unit::create(static_cast<UNIT_TYPE>(unit_type), location, player_id);
    return "Unit created";
  }

  Unit* generate_path(const fbs::MoveStep* move_step) {
    bool avoid_city = move_step->avoid_city();
    bool avoid_unit = move_step->avoid_unit();
    bool require_ownership = move_step->require_ownership();
    sf::Vector3i destination = get_v3i(move_step->destination());
    uint32_t unitId = move_step->unit_id();
    uint32_t player_id = move_step->player();

    auto find_tiles = [avoid_city, avoid_unit, require_ownership, player_id](const Tile& tile) -> bool {
      if (avoid_city && tile.m_city_id) return false;
      if (avoid_unit && !tile.m_unit_ids.empty()) return false;
      if (require_ownership) {
        uint32_t owner = world_map::tile_owner(tile);
        return owner == unique_id::INVALID_PLAYER || owner == player_id;
      }
      return true;
    };
    Player* player = player::get_player(player_id);
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
    if (player->m_ai_type != AI_TYPE::HUMAN && path.empty()) {
      unit->m_action_points = 0;
      std::cout << "AI attempted an empty path: unit exhausted." << std::endl;
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

  void execute_move(const fbs::MoveStep* move_step) {
    std::cout << "Executing immediate movement " << std::endl;
    // Just set where the unit needs to move and add it to a list. The actual move will happen in the move phase

    Unit* unit = generate_path(move_step);
    if (!unit) return;

    if (move_step->immediate()) {
      UnitMovementVector units_to_move;
      units_to_move.push_back(unit->m_id);
      while (step_move(units_to_move, unit->m_owner_id)) {
      }
    }
    // Queue it for continued movement, unit will remove itself if it is done moving
    s_units_to_move.push_back(unit->m_id);
  }

  std::string execute_purchase(const fbs::PurchaseStep* purchase_step) {
    uint32_t player_id = purchase_step->player();
    uint32_t city_id = purchase_step->city();
    uint32_t production_id = purchase_step->production();

    Player* player = player::get_player(player_id);
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(city_id);
    if(!city) return "Invalid City";
    if (production_id != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(production_id));
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

  std::string execute_research(const fbs::ResearchStep* research_step) {
    uint32_t player_id = research_step->player();
    uint32_t science = research_step->science();
    Player* player = player::get_player(player_id);
    if (!player) return "Invalid player";
    std::vector<uint32_t>& research = player->m_available_research;
    std::vector<uint32_t>::const_iterator findIt = find(research.begin(), research.end(), science);
    if (findIt == research.end()) return "Research is not available to player.";
    player->m_research = static_cast<SCIENCE_TYPE>(science);
    return "Research assigned.";
  }

  std::string execute_sell(const fbs::SellStep* sell_step) {
    uint32_t player_id = sell_step->player();
    uint32_t city_id = sell_step->city();
    uint32_t production_id = sell_step->production_id();
    Player* player = player::get_player(player_id);
    std::stringstream ss;
    if(!player) return "Invalid Player";
    City* city = city::get_city(city_id);
    if(!city) return "Invalid City";
    std::vector<CONSTRUCTION_TYPE> completed = production_queue::complete(city->GetProductionQueue());
    if (production_id != 0) {
      CONSTRUCTION_TYPE t(util::uint_to_enum<CONSTRUCTION_TYPE>(production_id));
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

  std::string execute_siege(const fbs::SiegeStep* siege_step) {
    uint32_t player_id = siege_step->player();
    uint32_t city_id = siege_step->city();
    uint32_t unit_id = siege_step->unit();
    Player* player = player::get_player(player_id);
    if(!player) return "Invalid Player";
    City* city = city::get_city(city_id);
    if(!city) return "Invalid City";
    Unit* unit = unit::get_unit(unit_id);
    if (!unit) return "Invalid Unit";
    if (!player->OwnsUnit(unit->m_id)) return "Player doesn't own unit";
    if (city->m_owner_id == player->m_id) return "Can't siege your own city";
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

  void execute_magic(const fbs::MagicStep* magic_step) {
    uint32_t player_id = magic_step->player();
    fbs::MAGIC_TYPE type = magic_step->type();
    sf::Vector3i location = get_v3i(magic_step->location());
    bool cheat = magic_step->cheat();
    magic::cast(player_id, (MAGIC_TYPE)type, location, cheat);
  }

  void execute_status(const fbs::StatusStep* status_step) {
    uint32_t type = status_step->type();
    sf::Vector3i location = get_v3i(status_step->location());
    status_effect::create((STATUS_TYPE)type, location);
  }

  void execute_scenario(const fbs::ScenarioStep* scenario_step) {
    scenario::start((SCENARIO_TYPE)scenario_step->type());
  }

  void execute_add_player(const fbs::AddPlayerStep* add_player_step) {
    fbs::AI_TYPE ai_type = add_player_step->ai_type();
    const flatbuffers::String* name = add_player_step->name();
    switch (ai_type) {
      case fbs::AI_TYPE::AI_TYPE_AI_BARBARIAN:
        player::create_ai((AI_TYPE)ai_type);
        break;
      case fbs::AI_TYPE::AI_TYPE_AI_HUMAN:
        player::create_human(name->str());
      default:
        break;
    }
  }

  void execute_attack(const fbs::AttackStep* attack_step) {
    // Add a fight to the list to be executed in the combat phase
    uint32_t player_id = attack_step->player(); 
    uint32_t attacker_id = attack_step->attacker_id();
    uint32_t defender_id = attack_step->defender_id();
    Player* player = player::get_player(player_id);
    if (!player) {
      std::cout << "Invalid player for attack action" << std::endl;
      return;
    }
    if (!player->OwnsUnit(attacker_id)) {
      std::cout << "Player does not own attacking unit" << std::endl;
      return;
    }
    s_units_to_fight.push_back(std::pair<uint32_t, uint32_t>(attacker_id, defender_id));
  }

  std::string execute_production_move(const fbs::ProductionMoveStep* move_step) {
    uint32_t player_id = move_step->player();
    uint32_t city_id = move_step->city();
    uint32_t src_index = move_step->source_index();
    uint32_t dest_index = move_step->destination_index();

    Player* player = player::get_player(player_id);
    if (!player) return "Invalid Player";
    City* city = city::get_city(city_id);
    if (!city) return "Invalid City";
    if (!player->OwnsCity(city->m_id)) return "Player doesn't own city.";
    production_queue::move(city->GetProductionQueue(), src_index, dest_index);
    return "Production move completed.";
  }

  std::string execute_production_abort(const fbs::ProductionAbortStep* abort_step) {
    uint32_t player_id = abort_step->player();
    uint32_t city_id = abort_step->city();
    uint32_t index = abort_step->index();

    Player* player = player::get_player(player_id);
    if (!player) return "Invalid Player";
    City* city = city::get_city(city_id);
    if (!city) return "Invalid City";
    if (!player->OwnsCity(city->m_id)) return "Player doesn't own city.";
    production_queue::remove(city->GetProductionQueue(), index);
    return "Removing...";
  }

  void execute_modify_stats(const fbs::UnitStatsStep* stats_step) {

    uint32_t unit_id = stats_step->unit_id();
    uint32_t health = stats_step->health();
    uint32_t attack = stats_step->attack();
    uint32_t range = stats_step->range();
    Unit* unit = unit::get_unit(unit_id);
    if (!unit) {
      return;
    }
    unit->m_combat_stats.m_health = static_cast<float>(health);
    unit->m_combat_stats.m_attack = static_cast<float>(attack);
    unit->m_combat_stats.m_range = static_cast<float>(range);

    std::cout << format::combat_stats(unit->m_combat_stats) << std::endl;
  }

  std::string execute_city_defense(const fbs::CityDefenseStep* city_defense) {
    uint32_t player_id = city_defense->player();
    uint32_t unit_id = city_defense->unit();
    Player* player = player::get_player(player_id);
    if(!player) return "Invalid Player";
    Unit* unit = unit::get_unit(unit_id);
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
    unit::damage(unit->m_id, player->m_id, 4.f);
    
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
  unit::reset();
  city::reset();
  science::reset();
  empire_trees::shutdown();
}

uint32_t simulation::get_turn() {
  return s_current_turn;
}

void simulation::process_step(const void* buffer, size_t buffer_len) {
  flatbuffers::Verifier v((uint8_t*)buffer, buffer_len);
  if (!fbs::VerifyAnyStepBuffer(v)) {
    std::cout << "Bad data arrived from the network buffer." << std::endl;
    return;
  }
  auto root = fbs::GetAnyStep(buffer);
  fbs::StepUnion t = root->step_type();

  // Process the step
  switch (t) {
    case fbs::StepUnion_QuitStep:
    break;
  case fbs::StepUnion_BeginStep:
    process_begin_turn();
    break;
  case fbs::StepUnion_EndTurnStep:
    process_end_turn(root->step_as_EndTurnStep());
    break;
  case fbs::StepUnion_AttackStep:
    execute_attack(root->step_as_AttackStep());
    break;
  case fbs::StepUnion_ProductionAbortStep:
    std::cout << execute_production_abort(root->step_as_ProductionAbortStep()) << std::endl;
    break;
  case fbs::StepUnion_ProductionMoveStep:
    std::cout << execute_production_move(root->step_as_ProductionMoveStep()) << std::endl;
    break;
  case fbs::StepUnion_ColonizeStep:
    execute_colonize(root->step_as_ColonizeStep());
    break;
  case fbs::StepUnion_ConstructionStep:
    execute_construction(root->step_as_ConstructionStep());
    break;
  case fbs::StepUnion_GrantStep:
    execute_grant(root->step_as_GrantStep());
    break;
  case fbs::StepUnion_HarvestStep:
    execute_harvest(root->step_as_HarvestStep());
    break;
  case fbs::StepUnion_ImproveStep:
    execute_improve(root->step_as_ImproveStep());
    break;
  case fbs::StepUnion_TileMutatorStep:
    execute_tile_mutator(root->step_as_TileMutatorStep());
    break;
  case fbs::StepUnion_ResourceMutatorStep:
    execute_resource_mutator(root->step_as_ResourceMutatorStep());
    break;
  case fbs::StepUnion_KillStep:
    execute_kill(root->step_as_KillStep());
    break;
  case fbs::StepUnion_MoveStep:
    execute_move(root->step_as_MoveStep());
    break;
  case fbs::StepUnion_PillageStep:
    std::cout << execute_pillage(root->step_as_PillageStep()) << std::endl;;
    break;
  case fbs::StepUnion_PurchaseStep:
    std::cout << execute_purchase(root->step_as_PurchaseStep()) << std::endl;
    break;
  case fbs::StepUnion_ResearchStep:
    std::cout << execute_research(root->step_as_ResearchStep()) << std::endl;
    break;
  case fbs::StepUnion_SpecializeStep:
    execute_specialize(root->step_as_SpecializeStep());
    break;
  case fbs::StepUnion_SellStep:
    std::cout << execute_sell(root->step_as_SellStep()) << std::endl;
    break;
  case fbs::StepUnion_SiegeStep:
    std::cout << execute_siege(root->step_as_SiegeStep()) << std::endl;
    break;
  case fbs::StepUnion_SpawnStep:
    std::cout << execute_spawn(root->step_as_SpawnStep()) << std::endl;
    break;
  case fbs::StepUnion_AddPlayerStep:
    execute_add_player(root->step_as_AddPlayerStep());
    return; // Special case, adding a player does not have output
  case fbs::StepUnion_UnitStatsStep:
    execute_modify_stats(root->step_as_UnitStatsStep());
    return; // Modifying stats also does not have output
  case fbs::StepUnion_BarbarianStep:
    return;
  case fbs::StepUnion_CityDefenseStep:
    std::cout << execute_city_defense(root->step_as_CityDefenseStep()) << std::endl;
    return;
  case fbs::StepUnion_MagicStep:
    execute_magic(root->step_as_MagicStep());
    break;
  case fbs::StepUnion_StatusStep:
    execute_status(root->step_as_StatusStep());
    break;
  case fbs::StepUnion_ScenarioStep:
    execute_scenario(root->step_as_ScenarioStep());
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

  phase_city_growth();
  phase_city_raze();
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

void simulation::process_end_turn(const fbs::EndTurnStep* end_turn) {
  uint32_t player_id = end_turn->player();
  Player* player = player::get_player(player_id);
  if (!player) return;

  if (player->m_ai_type == AI_TYPE::BARBARIAN) {
    barbarians::pillage_and_plunder(player_id);
  }

  if (player->m_ai_type == AI_TYPE::MONSTER) {
    monster::execute_turn(player_id);
  }

  phase_queued_movement(player_id);
  player->m_turn_state = TURN_TYPE::TURNCOMPLETED;
  if (player::all_players_turn_ended()) {
    process_begin_turn(); 
  }

  std::cout << std::endl << "Active player is " << end_turn->next_player()  << std::endl;
  NotificationVector events = notification::get_player_notifications(end_turn->next_player());
  for (size_t i = 0; i < events.size(); ++i) {
    std::cout << notification::to_string(events[i]) << std::endl;
  }
}
