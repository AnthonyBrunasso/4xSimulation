#include "terminal.h"

#include "city.h"
#include "format.h"
#include "step.h"
#include "tile.h"
#include "util.h"
#include "world_map.h"
#include "search.h"
#include "step_parser.h"
#include "hex.h"
#include "unit_definitions.h"
#include "game_types.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <fstream>

namespace terminal  {

// Return true because it was a valid query command but invalid arguments.
// Returning true will enforce that the query will not attempt to be parsed as a step.
#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return true; \
  } \
} 

  typedef std::unordered_map<std::string, std::function<bool(const std::vector<std::string>&)> > CommandMap;
  std::vector<std::string> s_help_list;
  CommandMap s_query_map;
  std::fstream s_target_file;

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
      sf::Vector3i start = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      int32_t distance = std::stoi(tokens[4]);
      std::vector<sf::Vector3i> coords;
      search::range(start, distance, coords);
      for (uint32_t i = 0; i < coords.size(); ++i) {
        Tile* near = world_map::get_tile(coords[i]);
        if (!near) continue;
        std::cout << "location: " << format::vector3(coords[i]) << " tile: " << format::tile(*near) << std::endl;
      }
      return true;
    });

    terminal::add_query("route", "route <xs> <ys> <zs> <xt> <yt> <zt>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(7, tokens);
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
      units::for_each_unit([](const Unit& unit) {
        std::cout << format::unit(unit) << std::endl;
      });
      return true;
    });

    terminal::add_query("unit", "unit <unitId>", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(2, tokens);
      Unit* unit = units::get_unit(std::stoul(tokens[1]));
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
      std::vector<sf::Vector3i> path;
      search::path_to(util::str_to_vector3(tokens[1], tokens[2], tokens[3]), 
        util::str_to_vector3(tokens[4], tokens[5], tokens[6]),
        world_map::get_map(), 
        path);

      std::cout << "Path size: " << path.size() << std::endl;
      for (auto node : path) {
        std::cout << format::vector3(node) << " ";
      }
      std::cout << std::endl;
      return true;
    });

    terminal::add_query("definitions", "definitions", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      unit_definitions::for_each_definition([](UNIT_TYPE type, const CombatStats& stats) {
        std::cout << get_unit_name(type) << ": " << format::combat_stats(stats) << std::endl;
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
      for_each_terrain_type([](TERRAIN_TYPE terrain) {
        std::cout << static_cast<int32_t>(terrain) << ": " << get_terrain_name(terrain) << std::endl;
      });
      return true;
    });

    terminal::add_query("resource_types", "resource_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_resource_type([](RESOURCE_TYPE resource) {
        std::cout << static_cast<int32_t>(resource) << ": " << get_resource_name(resource) << std::endl;
      });
      return true;
    });   
    
    terminal::add_query("improvement_types", "improvement_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_improvement_type([](IMPROVEMENT_TYPE improvement) {
        std::cout << static_cast<int32_t>(improvement) << ": " << get_improvement_name(improvement) << std::endl;
      });
      return true;
    });   
    
    terminal::add_query("unit_types", "unit_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_unit_type([](UNIT_TYPE unit) {
        std::cout << static_cast<int32_t>(unit) << ": " << get_unit_name(unit) << std::endl;
      });
      return true;
    });   
    
    terminal::add_query("building_types", "building_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_building_type([](BUILDING_TYPE building) {
        std::cout << static_cast<int32_t>(building) << ": " << get_building_name(building) << std::endl;
      });
      return true;
    });   
    
    terminal::add_query("construction_types", "construction_types", [](const std::vector<std::string>& tokens) -> bool {
      CHECK_VALID(1, tokens);
      for_each_construction_type([](CONSTRUCTION_TYPE construction) {
        std::cout << static_cast<int32_t>(construction) << ": " << get_construction_name(construction) << std::endl;
      });
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
    std::cout << "  begin_turn" << std::endl;
    std::cout << "  join <name>" << std::endl;
    std::cout << "  kill <unitId>" << std::endl;
    std::cout << "  spawn <unitType> <x> <y> <z>" << std::endl;
    std::cout << "  stats <unitId> <health> <attack> <range>"<< std::endl;
    std::cout << "  tile_cost <x> <y> <z> <cost>" << std::endl;
    std::cout << "  tile_resource <resourceType> <x> <y> <z> <quantity>" << std::endl;
    std::cout << "  quit" << std::endl;
    std::cout << "Player Commands: " << std::endl;
    std::cout << "  attack <attacker unitId> <defender unitId>" << std::endl;
    std::cout << "  colonize <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  construct <cityId> <productionType>" << std::endl;
    std::cout << "  discover <x> <y> <z>" << std::endl;
    std::cout << "  end_turn" << std::endl;
    std::cout << "  improve <improvementType> <x> <y> <z>" << std::endl;
    std::cout << "  move <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  queue_move <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  purchase <cityId> <buildingId>" << std::endl;
    std::cout << "  purchase <cityId> <unitId>" << std::endl;
    std::cout << "  sell <buildingId>" << std::endl;
    std::cout << "  sell <unitId>" << std::endl;
    // Modifies the stats of a unit
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

Step* terminal::parse_input() {
  Step* step = nullptr;

  // Get input until we've created a valid step
  while (!step) {
    std::cout << std::endl;
    std::string value;
    std::cout << "> ";
    std::getline(std::cin, value);
    if (!std::cin.good()) {
      return nullptr;
    }
    std::cout << "Executing: " << value << std::endl;

    std::vector<std::string> tokens;
    step_parser::split_to_tokens(value, tokens);
    // Execute any valid queries based on the tokens, if 
    // tokens make a query continue to next iteration.
    // Therefore, no command can be both a query and a step
    if (execute_queries(tokens)) continue;
    // Generate any valid steps from the tokens
    step = step_parser::parse(tokens);
    // If a valid step and not a quit, save the command to file.
    if (step && step->m_command != COMMAND::QUIT) {
      s_target_file << value << std::endl; 
    } 
  }

  return step;
}

void terminal::record_steps(const std::string& filename) {
  s_target_file.open(filename, std::ios::out);
}

void terminal::kill() {
  if (s_target_file.is_open()) {
    s_target_file.close();
  }
}
