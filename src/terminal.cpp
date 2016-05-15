#include "terminal.h"

#include "format.h"
#include "step.h"
#include "tile.h"
#include "util.h"
#include "world_map.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

namespace {

#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(); \
    return; \
  } \
}

#define CREATE_GENERIC_STEP(arg_count, tokens, step, COMMAND) { \
  CHECK_VALID(arg_count, tokens); \
  step = new Step(COMMAND); \
  return; \
}

  void parse_tokens(const std::vector<std::string>& tokens, Step*& step);
  void execute_help();
  void bad_arguments();

  void parse_tokens(const std::vector<std::string>& tokens, Step*& step) {
    if (!tokens.size()) {
      return;
    }

    // COMMANDS

    if (tokens[0] == "help") {
      execute_help();
      return;
    }

    if (tokens[0] == "quit") {
      CREATE_GENERIC_STEP(1, tokens, step, COMMAND::QUIT);
    }

    if (tokens[0] == "begin") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::BEGIN_TURN);
    }

    if (tokens[0] == "end") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::END_TURN);
    }

    if (tokens[0] == "attack") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::ATTACK);
    }

    if (tokens[0] == "colonize") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::COLONIZE);
    }

    if (tokens[0] == "construct") {
      CREATE_GENERIC_STEP(3, tokens, step, COMMAND::CONSTRUCT);
    }

    if (tokens[0] == "discover") {
      CREATE_GENERIC_STEP(4, tokens, step, COMMAND::DISCOVER);
    }

    if (tokens[0] == "improve") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::IMPROVE);
    }

    if (tokens[0] == "kill") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::KILL);
    }

    if (tokens[0] == "move") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::MOVE);
    }

    if (tokens[0] == "purchase") {
      CREATE_GENERIC_STEP(3, tokens, step, COMMAND::PURCHASE);
    }

    if (tokens[0] == "sell") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::SELL);
    }

    if (tokens[0] == "spawn") {
      CHECK_VALID(5, tokens);
      step = new SpawnStep(COMMAND::SPAWN);
      SpawnStep* spawn_step = static_cast<SpawnStep*>(step);
      spawn_step->m_uintId = std::stoul(tokens[1]);
      spawn_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
    }

    // QUERIES

    if (tokens[0] == "tiles") {
      
    } 

    if (tokens[0] == "tile") {
      CHECK_VALID(4, tokens);
      sf::Vector3i key = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      Tile* tile = world_map::get_tile(key);
      if (!tile) {
        std::cout << "tile: " << format::vector3(key) << " does not exist" << std::endl;
        return;
      }
      std::cout << format::tile(*tile) << std::endl; 
    }

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
    std::cout << "  move <unitId> <x> <y> <z>" << std::endl;
    std::cout << "  purchase <cityId> <buildingId>" << std::endl;
    std::cout << "  purchase <cityId> <unitId>" << std::endl;
    std::cout << "  sell <buildingId>" << std::endl;
    std::cout << "  sell <unitId>" << std::endl;
    std::cout << "  spawn <unitId> <x> <y> <z>" << std::endl << std::endl;

    std::cout << "Queries: " << std::endl;
    std::cout << "  tiles" << std::endl;
    std::cout << "  tile <x> <y> <z>" << std::endl;
  }

  void bad_arguments() {
    std::cout << "Invalid arguments" << std::endl;
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

    std::stringstream iss(value);
    std::vector<std::string> tokens;

    // Split string on space
    std::copy(std::istream_iterator<std::string>(iss),
      std::istream_iterator<std::string>(),
      std::back_inserter(tokens));

    parse_tokens(tokens, step); 
  }

  return step;
}