#include "terminal.h"

#include <ext/alloc_traits.h>
#include <stdint.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>

#include "Vector3.hpp"
#include "city.h"
#include "format.h"
#include "game_types.h"
#include "hex.h"
#include "improvement.h"
#include "player.h"
#include "production.h"
#include "random.h"
#include "scenario.h"
#include "science.h"
#include "search.h"
#include "simulation.h"
#include "status_effect.h"
#include "step_parser.h"
#include "terrain_yield.h"
#include "tile.h"
#include "unit.h"
#include "unit_definitions.h"
#include "util.h"
#include "world_map.h"

class CombatStats;
struct Resource;

namespace terminal  {

// Return true because it was a valid query command but invalid arguments.
// Returning true will enforce that the query will not attempt to be parsed as a step.
#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return true; \
  } \
} 

#define CHECK(arg_count, tokens) { \
  if (tokens.size() < arg_count) { \
    bad_arguments(tokens); \
    return true; \
  } \
}

  typedef std::unordered_map<std::string, std::function<bool(const std::vector<std::string>&)> > CommandMap;
  std::vector<std::string> s_help_list;
  CommandMap s_query_map;

  bool execute_queries(const std::vector<std::string>& tokens);
  void execute_help();
  void bad_arguments(const std::vector<std::string>& tokens);
  void draw_tile(const sf::Vector3i coord);

  void initialize() {
    terminal::add_query("help", "help", [](const std::vector<std::string>&) -> bool {
      execute_help();
      return true;
    });

    terminal::add_query("cities", "cities", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      city::for_each_city([](City& city) {
        std::cout << format::city(city) << std::endl;
      });
      return true;
    });

    terminal::add_query("city", "city <cityId>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(2, tokens);
      City* city = city::get_city(std::stoul(tokens[1]));
      if (!city) {
        std::cout << "city: " << tokens[1] << " does not exist" << std::endl;
        return true;
      }
      std::cout << format::city(*city);
      return true;
    });

    terminal::add_query("undiscovered", "undiscovered", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      world_map::for_each_tile([](const sf::Vector3i& coord, const Tile& tile) {
        if (tile.m_discover_bonus) {
          std::cout << "Tile at " << format::vector3(coord) << " is undiscovered" << std::endl;
        }
      });
      return true;
    });

    terminal::add_query("science", "science <playerId> [scienceType]", [](const std::vector<std::string>& tokens) -> bool {
      CHECK(2, tokens);
      uint32_t player_id = std::stoul(tokens[1]);
      if (tokens.size() > 2) {
      fbs::SCIENCE_TYPE st = util::enum_from_names<fbs::SCIENCE_TYPE>(tokens[2], fbs::EnumNamesSCIENCE_TYPE());
        if (st == fbs::SCIENCE_TYPE::UNKNOWN) {
          st = static_cast<fbs::SCIENCE_TYPE>(std::stoul(tokens[2]));
        }
        ScienceNode* sn = science::Science(st);
        if (!sn) return false;

        science::debug_requirements(sn);
      
        uint32_t player_id = std::stoul(tokens[1]);
        if (science::available(player_id, sn)) {
          std::cout << "Science is ready for research" << std::endl;
        }
        else {
          std::cout << "The study of this science is yet unknown to this player." << std::endl;
        }
        
        std::cout << "The science costs: " << science::research_cost(sn) << std::endl;
      }

      Player* player = player::get_player(player_id);
      if (!player) return false;
      std::cout << "-- Available Sciences --" << std::endl;
      for (size_t i = 0; i < player->m_available_research.size(); ++i) {
        uint32_t st_i = player->m_available_research[i];
        ScienceNode* sn = science::Science(st_i);
        if (!sn) continue;
        if (sn->m_type == player->m_research) {
          std::cout << "*";
        }
        std::cout << static_cast<uint32_t>(sn->m_type) << ": " << sn->Name() << std::endl;
      }
      
      return true;
    });
    
    terminal::add_query("idle_queue", "idle_queue", [](const std::vector<std::string>& tokens) {
      CHECK_VALID(1, tokens);
  
      Player* p = player::get_player(step_parser::get_active_player_id());
      if (!p) return false;
      
      bool stop = false;
      city::for_each_city([&stop, p] (City& city) {
        if (stop) return;
        if (p->m_id != city.m_owner_id) return;
        if (city.IsConstructing()) return;
        std::vector<fbs::CONSTRUCTION_TYPE> incomplete = production_queue::incomplete(city.GetProductionQueue());
        std::cout << "City (" << city.m_id << ") construct " << city.m_id << " <constructionType>" << std::endl;
        for (size_t i = 0; i < incomplete.size(); ++i) {
          fbs::CONSTRUCTION_TYPE t = incomplete[i];
          std::cout <<  static_cast<uint32_t>(t) << " " << fbs::EnumNameCONSTRUCTION_TYPE(t) << std::endl;
        }
        stop = true;
      });
      return true;
    });
    
    terminal::add_query("idle_worker", "idle_worker <distance>", [](const std::vector<std::string>& tokens) {
      CHECK_VALID(1, tokens);
      
      uint32_t distance = 1;
      if (tokens.size() > 1) {
        distance =  std::stoul(tokens[1]);
      }

      if (distance > 3) return false;
 
      Player* p = player::get_player(step_parser::get_active_player_id());
      if (!p) return false;

      bool stop = false;
      city::for_each_city([p, distance, &stop](const City& city) {
        if (stop) return;
        if (p->m_id != city.m_owner_id) return;
        if (city.IdleWorkers() == 0) return;
        std::cout << "City (" << city.m_id << ") harvest <x> <y> <z>" << std::endl;
        std::vector<sf::Vector3i> coords;
        std::set<uint32_t> terrainTypes;
        coords = search::range(city.m_location, distance);
        for (size_t i = 0; i < coords.size(); ++i) {
          Tile* t = world_map::get_tile(coords[i]);
          if(!t) continue;
          if(terrain_yield::is_harvested(coords[i])) continue;
          std::cout << "  coord: " << format::vector3(coords[i]) << "  " << terrain_yield::get_yield(coords[i], city.m_specialization) << std::endl;
        }
        stop = true;
      });
      return true;
    });
 
    terminal::add_query("idle_unit", "idle_unit", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      bool stop = false;
      player::for_each_player_unit(step_parser::get_active_player_id(), [&stop](Unit& unit) {
        if(stop) return;
        if (unit.m_path.empty()) {
          std::cout << format::unit(unit) << std::endl;
          stop = true;
        }
      });
      return true;
    });

    terminal::add_query("tiles", "tiles", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      world_map::for_each_tile([](const sf::Vector3i& coord, const Tile& tile) {
        std::cout << format::vector3(coord) << ": " << format::tile(tile) << std::endl;
      });
      return true;
    });

    terminal::add_query("tile", "tile <x> <y> <z>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(4, tokens);
      sf::Vector3i key = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      Tile* tile = world_map::get_tile(key);
      if (!tile) {
        std::cout << "tile: " << format::vector3(key) << " does not exist" << std::endl;
        return true;
      }
      std::cout << format::tile(*tile) << std::endl; 
      return true;
    });

    terminal::add_query("players", "players", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      int count = 0;
      player::for_each_player([&count](Player& player) {
        std::cout << count << ": " << format::player(player) << std::endl;
        ++count;
      });
      return true;
    });

    terminal::add_query("range", "range <x> <y> <z> <n>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(5, tokens);
      Tile* tile = world_map::get_tile(util::str_to_vector3(tokens[1], tokens[2], tokens[3]));
      if (!tile) return false;
      
      sf::Vector3i start = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      int32_t distance = std::stoi(tokens[4]);
      // 500 is already 751k tiles
      if (distance > 500) {
        std::cout << "distance limit of 500 applies to range" << std::endl;
        return false;
      }
      std::vector<sf::Vector3i> coords;
      coords = search::range(start, distance);
      for (uint32_t i = 0; i < coords.size(); ++i) {
        Tile* near = world_map::get_tile(coords[i]);
        if (!near) continue;
        std::cout << "location: " << format::vector3(coords[i]) << " tile: " << format::tile(*near) << std::endl;
      }
      return true;
    });

    terminal::add_query("route", "route <xs> <ys> <zs> <xt> <yt> <zt>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(7, tokens);
      // Verify they are valid world coordinates first
      Tile* tileA = world_map::get_tile(util::str_to_vector3(tokens[1], tokens[2], tokens[3]));
      if (!tileA) return false;
      Tile* tileB = world_map::get_tile(util::str_to_vector3(tokens[4], tokens[5], tokens[6]));
      if (!tileB) return false;
      // Shows the route from start to end, inclusive
      sf::Vector3f start = util::str_to_vector3f(tokens[1], tokens[2], tokens[3]);
      sf::Vector3f end = util::str_to_vector3f(tokens[4], tokens[5], tokens[6]);
      std::vector<sf::Vector3i> route;
      hex::cubes_on_line(start, end, route);
      for (uint32_t i = 0; i < route.size(); ++i) {
        std::cout << format::vector3(route[i]) << std::endl;
      }
      return true;
    });

    terminal::add_query("units", "units", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      unit::for_each_unit([](const Unit& unit) {
        std::cout << format::unit(unit) << std::endl;
      });
      return true;
    });

    terminal::add_query("unit", "unit <unitId>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(2, tokens);
      Unit* unit = unit::get_unit(std::stoul(tokens[1]));
      if (!unit) {
        std::cout << "id: " << tokens[1] << " does not exist" << std::endl;
        return true;
      }
      std::cout << format::unit(*unit) << std::endl;
      return true;
    });

    terminal::add_query("draw", "draw tile <x> <y> <z>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(5, tokens);
      draw_tile(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      return true;
    });
    
    terminal::add_query("path_to", "path_to <x> <y> <z> <tox> <toy> <toz>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(7, tokens);
      std::vector<sf::Vector3i> path = search::path_to(util::str_to_vector3(tokens[1], tokens[2], tokens[3]), 
        util::str_to_vector3(tokens[4], tokens[5], tokens[6]),
        world_map::get_map());

      std::cout << "Path size: " << path.size() << std::endl;
      for (auto node : path) {
        std::cout << format::vector3(node) << " ";
      }
      std::cout << std::endl;
      return true;
    });

    terminal::add_query("definitions", "definitions", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      unit_definitions::for_each_definition([](fbs::UNIT_TYPE type, const CombatStats& stats) {
        std::cout << fbs::EnumNameUNIT_TYPE(type) << ": " << format::combat_stats(stats) << std::endl;
      });
      return true;
    });

    terminal::add_query("improvements", "improvements", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      improvement::for_each_improvement([](const Improvement& improvement) {
        std::cout << format::improvement(improvement) << std::endl;
      });
      return true;
    });

    terminal::add_query("terrain_types", "terrain_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      auto check = ([](fbs::TERRAIN_TYPE terrain) {
        std::cout << static_cast<int32_t>(terrain) << ": " << fbs::EnumNameTERRAIN_TYPE(terrain) << std::endl;
      });
      for (auto tt : fbs::EnumValuesTERRAIN_TYPE()) {
        check(tt);
      }
      return true;
    });

    terminal::add_query("resource_types", "resource_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      auto check = ([](fbs::RESOURCE_TYPE resource) {
        std::cout << static_cast<int32_t>(resource) << ": " << fbs::EnumNameRESOURCE_TYPE(resource) << std::endl;
      });
      for (auto rt : fbs::EnumValuesRESOURCE_TYPE()) {
        check(rt);
      }
      return true;
    });   
    
    terminal::add_query("improvement_types", "improvement_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_improvement_type([](IMPROVEMENT_TYPE improvement) {
        std::cout << static_cast<int32_t>(improvement) << ": " << get_improvement_name(improvement) << std::endl;
        std::cout << "  " << improvement::resource_requirements(improvement).size() << " requirements" << std::endl;
      });
      return true;
    });   
    
    terminal::add_query("unit_types", "unit_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      auto check = ([](fbs::UNIT_TYPE unit) {
        std::cout << static_cast<int32_t>(unit) << ": " << fbs::EnumNameUNIT_TYPE(unit) << std::endl;
      });
      for (auto ut : fbs::EnumValuesUNIT_TYPE()) {
        check(ut);
      }
      return true;
    });   
    
    terminal::add_query("building_types", "building_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      auto check = ([](fbs::BUILDING_TYPE building) {
        std::cout << static_cast<int32_t>(building) << ": " << fbs::EnumNameBUILDING_TYPE(building) << std::endl;
      });
      for (auto bt : fbs::EnumValuesBUILDING_TYPE()) {
        check(bt);
      }
      return true;
    });   
    
    terminal::add_query("construction_types", "construction_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      auto check = ([](fbs::CONSTRUCTION_TYPE construction) {
        std::cout << static_cast<int32_t>(construction) << ": " << fbs::EnumNameCONSTRUCTION_TYPE(construction) << std::endl;
      });
      for (auto ct : fbs::EnumValuesCONSTRUCTION_TYPE()) {
        check(ct);
      }
      return true;
    });

    terminal::add_query("search", "search <x> <y> <z> <depth>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(5, tokens);
      sf::Vector3i start = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      uint32_t depth = std::stoul(tokens[4]);
      // Just bfs for a unit in range.
      auto contains_unit = [](const Tile& tile) {
        return !tile.m_unit_ids.empty();
      };
      if (search::bfs(start, depth, world_map::get_map(), contains_unit)) {
        std::cout << "Unit found in depth: " << depth << " from: " << format::vector3(start) << std::endl;
      }
      else {
        std::cout << "Unit not found." << std::endl;
      }
      return true; 
    });

    terminal::add_query("search_type", "search_type <type> <x> <y> <z> <depth>", [](const std::vector<std::string>& tokens) -> bool{
      CHECK_VALID(6, tokens);
      fbs::SEARCH_TYPE type = util::enum_from_names<fbs::SEARCH_TYPE>(tokens[1], fbs::EnumNamesSEARCH_TYPE());
      sf::Vector3i start = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      uint32_t depth = std::stoul(tokens[5]);
      static auto s_units = [](const Unit& u) -> bool {
        std::cout << format::unit(u) << std::endl;
        return false;
      };
      static auto s_cities = [](const City& c) -> bool {
        std::cout << format::city(c) << std::endl;
        return false;
      };
      static auto s_improvements = [](const Improvement& i) -> bool {
        std::cout << format::improvement(i) << std::endl;
        return false;
      };
      static auto s_resources = [](const Resource& r) -> bool {
        std::cout << format::resource(r) << std::endl;
        return false;
      };
      switch (type) {
      case fbs::SEARCH_TYPE::UNITS:
        search::bfs_units(start, depth, world_map::get_map(), s_units);
        return true;
      case fbs::SEARCH_TYPE::CITIES:
        search::bfs_cities(start, depth, world_map::get_map(), s_cities);
        return true;
      case fbs::SEARCH_TYPE::IMPROVEMENTS:
        search::bfs_improvements(start, depth, world_map::get_map(), s_improvements);
        return true;
      case fbs::SEARCH_TYPE::RESOURCES:
        search::bfs_resources(start, depth, world_map::get_map(), s_resources);
        return true;
      case fbs::SEARCH_TYPE::UNKNOWN:
      default:
        std::cout << "Unknown search type." << std::endl;
        return true;
      }
      return true;
    });

    terminal::add_query("seed", "seed <value>", [](const std::vector<std::string>& tokens) -> bool{
      CHECK_VALID(2, tokens);
      game_random::set_seed(std::stoul(tokens[1]));
      return true;
    });

    terminal::add_query("status_effects", "status_effects", [](const std::vector<std::string>& tokens) -> bool{
      CHECK_VALID(1, tokens);
      status_effect::for_each_effect([](const StatusEffect& effect) {
        std::cout << format::effect(effect) << std::endl;
      });
      return true;
    });

    terminal::add_query("scenario_debug", "scenario_debug <scenario_type>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(2, tokens);
      fbs::SCENARIO_TYPE type = util::enum_from_names<fbs::SCENARIO_TYPE>(tokens[1], fbs::EnumNamesSCENARIO_TYPE());
      scenario::debug_print(type);
      return true;
    });

    terminal::add_query("reset", "reset", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      simulation::reset();
      return true;
    });
  }

  bool execute_queries(const std::vector<std::string>& tokens) {
    if (!tokens.size()) {
      return false;
    }

    auto operation = s_query_map.find(tokens[0]);
    if (operation == s_query_map.end()) {
      return false;
    }

    return operation->second(tokens);
  }

  void execute_help() {
    // Targets are represented by <x> <y> <z> cube coordinates
    std::cout << "Admin Commands: " << std::endl;
    std::cout << "  active_player <playerId>" << std::endl;
    std::cout << "  barbarians" << std::endl;
    std::cout << "  begin_turn" << std::endl;
    std::cout << "  join <name> [<aiType>]" << std::endl;
    std::cout << "  kill <unitId>" << std::endl;
    std::cout << "  scenario <scenarioType>" << std::endl;
    std::cout << "  spawn <unitType> <x> <y> <z>" << std::endl;
    std::cout << "  stats <unitId> <health> <attack> <range>"<< std::endl;
    std::cout << "  tile_cost <x> <y> <z> <cost>" << std::endl;
    std::cout << "  tile_resource <resourceType> <x> <y> <z> [1<quantity>]" << std::endl;
    std::cout << "  quit" << std::endl;
    std::cout << "Player Commands: " << std::endl;
    std::cout << "  attack <attacker unitId> <defender unitId>" << std::endl;
    std::cout << "  city_defense <unitId>" << std::endl;
    std::cout << "  cast <magicType> <x> <y> <z>" << std::endl;
    std::cout << "  colonize <x> <y> <z>" << std::endl;
    std::cout << "  construct <cityId> <constructionType>" << std::endl;
    std::cout << "  end_turn" << std::endl;
    std::cout << "  grant <scienceType>" << std::endl;
    std::cout << "  harvest <x> <y> <z>" << std::endl;
    std::cout << "  improve <resourceType> <x> <y> <z>" << std::endl;
    std::cout << "  move <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  pillage <uintId>" << std::endl;
    std::cout << "  production_abort <cityId> <constructionIndex>" << std::endl;
    std::cout << "  production_move <cityId> <sourceIndex> <destinationIndex>" << std::endl;
    std::cout << "  purchase <cityId> [constructionType]" << std::endl;
    std::cout << "  queue_move <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  sell <cityId> [constructionType]" << std::endl;
    std::cout << "  siege <cityId> <unitId>" << std::endl;
    std::cout << "  specialize <cityId> <terrain_type>" << std::endl;
    std::cout << "  status <statusType> <x> <y> <z>" << std::endl;
    
    std::cout << std::endl;
    std::cout << "Queries: " << std::endl;
    for (auto help : s_help_list) {
      std::cout << "  " << help << std::endl;
    }

    std::cout << std::endl << "ACII Drawing: " << std::endl;
    std::cout << "  draw tile <x> <y> <z>" << std::endl;
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid query: " << format::vector(tokens) << std::endl;
  }

  void draw_tile(sf::Vector3i coord) {
    Tile* tile = world_map::get_tile(coord);
    if (!tile) {
      std::cout << "Invalid tile: " << format::vector3(coord);
      return;
    }

    hex::CubeNeighbors adj(coord);

    std::string ce = format::ascii_tile(tile);
    std::string ne = format::ascii_tile(world_map::get_tile(adj[0]));
    std::string ea = format::ascii_tile(world_map::get_tile(adj[1]));
    std::string se = format::ascii_tile(world_map::get_tile(adj[2]));
    std::string sw = format::ascii_tile(world_map::get_tile(adj[3]));
    std::string we = format::ascii_tile(world_map::get_tile(adj[4]));
    std::string nw = format::ascii_tile(world_map::get_tile(adj[5]));

    std::cout << "Legend: " << std::endl;
    std::cout << "  NE - North East" << std::endl;
    std::cout << "  E  - East      " << std::endl;
    std::cout << "  SE - South East" << std::endl;
    std::cout << "  SW - South West" << std::endl;
    std::cout << "  W  - West      " << std::endl;
    std::cout << "  NW - North West" << std::endl;
    std::cout << "  C  - Center    " << std::endl;
    std::cout << "  *  - Unit      " << std::endl;
    std::cout << "  ^  - Building  " << std::endl;
    std::cout << " */^ - Unit and Building" << std::endl;
    std::cout << "           _____                " << std::endl;
    std::cout << "          /     \\              " << std::endl;
    std::cout << "         /       \\          W" << format::vector3(adj[4]) << std::endl;
    std::cout << "   ,----<   " << we << "   >----.        " << std::endl;
    std::cout << "  /      \\   W   /      \\     " << std::endl;
    std::cout << " /   " << sw << "  \\_____/   " << nw << "  \\  SW" << format::vector3(adj[3]) << " NW" << format::vector3(adj[5]) << std::endl;
    std::cout << " \\        /     \\        /    " << std::endl;
    std::cout << "  \\  SW  /       \\  NW  /     " << std::endl;
    std::cout << "   >----<   " << ce << "   >----<    C" << format::vector3(coord) << std::endl;
    std::cout << "  /      \\       /      \\     " << std::endl;
    std::cout << " /   " << se << "  \\_____/   " << ne << "  \\    " << std::endl;
    std::cout << " \\        /     \\        /  SE" << format::vector3(adj[2]) << "  NE" << format::vector3(adj[0]) << std::endl;
    std::cout << "  \\  SE  /       \\  NE  /     " << std::endl;
    std::cout << "   `----<   " << ea << "   >----'        " << std::endl;
    std::cout << "         \\   E   /          E" << format::vector3(adj[1]) << std::endl;
    std::cout << "          \\____ /            " << std::endl;       
  }
}

void terminal::add_query(
    const std::string& command, 
    const std::string& help,
    std::function<bool(const std::vector<std::string>&)> operation) {
  s_query_map[command] = operation;
  s_help_list.push_back(help);
  std::sort(s_help_list.begin(), s_help_list.end());
}

std::vector<std::string> terminal::tokenize(const std::string& input) {

  std::cout << "Tokenizing: " << input << std::endl;
  std::vector<std::string> tokens = step_parser::split_to_tokens(input);

  return std::move(tokens);
}

bool terminal::is_query(const std::vector<std::string> & tokens) {
  auto operation = s_query_map.find(tokens[0]);
  return operation != s_query_map.end();
}

bool terminal::run_query(const std::vector<std::string> & tokens) {
  return execute_queries(tokens);
}

bool terminal::run_step(const std::vector<std::string> & tokens, bool& game_over) {
  const size_t BUFFER_LEN = 512;
  char buffer[BUFFER_LEN];

  // See if a command is ready
  size_t bytes = step_parser::parse(tokens, buffer, BUFFER_LEN);

  if (!bytes) return false;

  // Process the step
  simulation::process_step(buffer, bytes);

  game_over = simulation::game_over();

  return true;
}

size_t terminal::step_to_bytes(const std::vector<std::string>& tokens, void* buffer, size_t buffer_len) {
  return step_parser::parse(tokens, buffer, buffer_len);
}

void terminal::kill() {
}
