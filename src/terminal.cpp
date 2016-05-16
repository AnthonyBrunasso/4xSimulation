#include "terminal.h"

#include "format.h"
#include "step.h"
#include "tile.h"
#include "util.h"
#include "world_map.h"
#include "search.h"
#include "step_parser.h"
#include "hex.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return false; \
  } \
} 

  bool execute_queries(const std::vector<std::string>& tokens);
  void execute_help();
  void bad_arguments(const std::vector<std::string>& tokens);
  void draw_tile(const sf::Vector3i coord);

  bool execute_queries(const std::vector<std::string>& tokens) {
    if (!tokens.size()) {
      return false;
    }

    if (tokens[0] == "help") {
      execute_help();
      return true;
    }

    else if (tokens[0] == "tiles") {
      CHECK_VALID(1, tokens);
      world_map::for_each_tile([](const sf::Vector3i& coord, const Tile& tile) {
        std::cout << format::vector3(coord) << ": " << format::tile(tile) << std::endl;
      });
      return true;
    } 

    else if (tokens[0] == "tile") {
      CHECK_VALID(4, tokens);
      sf::Vector3i key = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      Tile* tile = world_map::get_tile(key);
      if (!tile) {
        std::cout << "tile: " << format::vector3(key) << " does not exist" << std::endl;
        return true;
      }
      std::cout << format::tile(*tile) << std::endl; 
      return true;
    }

    else if (tokens[0] == "range") {
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
    }

    else if (tokens[0] == "units") {
      CHECK_VALID(1, tokens);
      units::for_each_unit([](const Unit& unit) {
        std::cout << format::unit(unit) << std::endl;
      });
      return true;
    }

    else if (tokens[0] == "unit") {
      CHECK_VALID(2, tokens);
      Unit* unit = units::get_unit(std::stoul(tokens[1]));
      if (!unit) {
        std::cout << "id: " << tokens[1] << " does not exist" << std::endl;
        return true;
      }

      std::cout << format::unit(*unit) << std::endl;
      return true;
    }

    else if (tokens[0] == "draw") {
      if (tokens[1] == "tile") {
        CHECK_VALID(5, tokens);
        draw_tile(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
        return true;
      }
    }

    return false;
  }

  void execute_help() {
    // Targets are represented by <x> <y> <z> cube coordinates
    std::cout << "Commands: " << std::endl;
    std::cout << "  help" << std::endl;
    std::cout << "  quit" << std::endl;
    std::cout << "  attack <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  begin turn" << std::endl;
    std::cout << "  colonize <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  construct <cityId> <buildingId>" << std::endl;
    std::cout << "  construct <cityId> <unitId>" << std::endl;
    std::cout << "  discover <x> <y> <z>" << std::endl;
    std::cout << "  end turn" << std::endl;
    std::cout << "  improve <x> <y> <z> <improvement>" << std::endl;
    std::cout << "  kill <unitId>" << std::endl;
    std::cout << "  move <unitId> (<x> <y> <z> OR nw OR ne OR e OR se OR sw OR w)" << std::endl;
    std::cout << "  purchase <cityId> <buildingId>" << std::endl;
    std::cout << "  purchase <cityId> <unitId>" << std::endl;
    std::cout << "  sell <buildingId>" << std::endl;
    std::cout << "  sell <unitId>" << std::endl;
    std::cout << "  spawn <unitId> <x> <y> <z>" << std::endl << std::endl;

    std::cout << "Queries: " << std::endl;
    std::cout << "  tiles" << std::endl;
    std::cout << "  tile <x> <y> <z>" << std::endl;
    std::cout << "  range <x> <y> <z> <n>" << std::endl;
    std::cout << "  units" << std::endl;
    std::cout << "  unit <unitId>" << std::endl << std::endl;

    std::cout << "ACII Drawing: " << std::endl;
    std::cout << "  draw tile <x> <y> <z>" << std::endl;
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::vector(tokens) << std::endl;
  }

  void draw_tile(sf::Vector3i coord) {
    Tile* tile = world_map::get_tile(coord);
    if (!tile) {
      std::cout << "Invalid tile: " << format::vector3(coord);
      return;
    }

    hex::CubeNeighbors adj(coord);

    std::cout << "Legend: " << std::endl;
    std::cout << "  NW - North West" << std::endl;
    std::cout << "  NE - North East" << std::endl;
    std::cout << "  E  - East      " << std::endl;
    std::cout << "  SE - South East" << std::endl;
    std::cout << "  SW - South West" << std::endl;
    std::cout << "  W  - West      " << std::endl;
    std::cout << "  C  - Center    " << std::endl;
    std::cout << "  *  - Unit      " << std::endl;
    std::cout << "  ^  - Building  " << std::endl;
    std::cout << "           _____                " << std::endl;
    std::cout << "          /     \\              " << std::endl;
    std::cout << "         /       \\          W" << format::vector3(adj[4]) << std::endl;
    std::cout << "   ,----<         >----.        " << std::endl;
    std::cout << "  /      \\   W   /      \\     " << std::endl;
    std::cout << " /        \\_____/        \\  SW" << format::vector3(adj[3]) << " NW" << format::vector3(adj[5]) << std::endl;
    std::cout << " \\        /     \\        /    " << std::endl;
    std::cout << "  \\  SW  /       \\  NW  /     " << std::endl;
    std::cout << "   >----<         >----<    C" << format::vector3(coord) << std::endl;
    std::cout << "  /      \\       /      \\     " << std::endl;
    std::cout << " /        \\_____/        \\    " << std::endl;
    std::cout << " \\        /     \\        /  SE" << format::vector3(adj[2]) << "  NE" << format::vector3(adj[0]) << std::endl;
    std::cout << "  \\  SE  /       \\  NE  /     " << std::endl;
    std::cout << "   `----<         >----'        " << std::endl;
    std::cout << "         \\   E   /          E" << format::vector3(adj[1]) << std::endl;
    std::cout << "          \\____ /            " << std::endl;       
  }
}

Step* terminal::parse_input() {
  Step* step = nullptr;

  // Get input until we've created a valid step
  while (!step) {
    std::cout << std::endl;
    std::string value;
    std::cout << "> ";
    std::getline(std::cin, value);

    std::vector<std::string> tokens;
    step_parser::split_to_tokens(value, tokens);
    // Execute any valid queries based on the tokens, if 
    // tokens make a query continue to next iteration.
    // Therefore, no command can be both a query and a step
    if (execute_queries(tokens)) continue;
    // Generate any valid steps from the tokens
    step = step_parser::parse(tokens); 
  }

  return step;
}