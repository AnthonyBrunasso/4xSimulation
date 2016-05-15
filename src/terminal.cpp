#include "terminal.h"

#include "format.h"
#include "step.h"
#include "tile.h"
#include "util.h"
#include "world_map.h"
#include "step_parser.h"

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
    std::cout << "  move <unitId> (<x> <y> <z> OR ne OR we OR se OR sw OR we OR nw)" << std::endl;
    std::cout << "  purchase <cityId> <buildingId>" << std::endl;
    std::cout << "  purchase <cityId> <unitId>" << std::endl;
    std::cout << "  sell <buildingId>" << std::endl;
    std::cout << "  sell <unitId>" << std::endl;
    std::cout << "  spawn <unitId> <x> <y> <z>" << std::endl << std::endl;

    std::cout << "Queries: " << std::endl;
    std::cout << "  tiles" << std::endl;
    std::cout << "  tile <x> <y> <z>" << std::endl;
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::tokens(tokens) << std::endl;
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