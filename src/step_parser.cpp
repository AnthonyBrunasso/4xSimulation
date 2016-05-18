#include "step_parser.h"

#include "step.h"
#include "util.h"
#include "format.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace {

#define CHECK(arg_count, tokens) { \
  if (tokens.size() < arg_count) { \
    bad_arguments(tokens); \
    return; \
  } \
}

#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return; \
  } \
}

#define CREATE_GENERIC_STEP(arg_count, tokens, step, COMMAND) { \
  CHECK_VALID(arg_count, tokens); \
  step = new Step(COMMAND); \
  return; \
}

  void parse_tokens(const std::vector<std::string>& tokens, Step*& step);
  void bad_arguments(const std::vector<std::string>& tokens);

  void parse_tokens(const std::vector<std::string>& tokens, Step*& step) {
    if (!tokens.size()) {
      return;
    }

    if (tokens[0] == "quit") {
      CREATE_GENERIC_STEP(1, tokens, step, COMMAND::QUIT);
    }

    else if (tokens[0] == "begin") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::BEGIN_TURN);
    }

    else if (tokens[0] == "end") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::END_TURN);
    }

    else if (tokens[0] == "attack") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::ATTACK);
    }

    else if (tokens[0] == "colonize") {
      CHECK(5, tokens);
      ColonizeStep* colonize_step = new ColonizeStep(COMMAND::COLONIZE);
      colonize_step->m_unit_id = std::stoul(tokens[1]);
      colonize_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      step = colonize_step;
      if (tokens.size() == 6) {
        colonize_step->m_player = std::stoul(tokens[5]);
      }
    }

    else if (tokens[0] == "construct") {
      CREATE_GENERIC_STEP(3, tokens, step, COMMAND::CONSTRUCT);
    }

    else if (tokens[0] == "discover") {
      CREATE_GENERIC_STEP(4, tokens, step, COMMAND::DISCOVER);
    }

    else if (tokens[0] == "improve") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::IMPROVE);
    }

    else if (tokens[0] == "join") {
      CHECK_VALID(2, tokens);
      AddPlayerStep* player_step = new AddPlayerStep(COMMAND::ADD_PLAYER);
      player_step->m_name = tokens[1];
      step = player_step;
    }

    else if (tokens[0] == "kill") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::KILL);
    }

    else if (tokens[0] == "move") {
      CHECK_VALID(5, tokens);
      MoveStep* move_step = new MoveStep(COMMAND::MOVE);
      move_step->m_unit_id = std::stoul(tokens[1]);
      move_step->m_destination = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      step = move_step;
    }

    else if (tokens[0] == "purchase") {
      CREATE_GENERIC_STEP(3, tokens, step, COMMAND::PURCHASE);
    }

    else if (tokens[0] == "sell") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::SELL);
    }

    else if (tokens[0] == "spawn") {
      CHECK(5, tokens);
      step = new SpawnStep(COMMAND::SPAWN);
      SpawnStep* spawn_step = static_cast<SpawnStep*>(step);
      spawn_step->m_entity_type = std::stoul(tokens[1]);
      spawn_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      // Optional player param, defaults to 0, or the 'player one'
      if (tokens.size() == 6) {
        spawn_step->m_player = std::stoul(tokens[5]);
      }
    }

    else {
      std::cout << "Unrecognized step: " << format::vector(tokens) << std::endl;
    }
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::vector(tokens) << std::endl;
  }
}

void step_parser::split_to_tokens(const std::string& line, std::vector<std::string>& tokens) {
  std::stringstream iss(line);
  // Make sure output vector is empty first
  tokens.clear();

  // Split string on space
  std::copy(std::istream_iterator<std::string>(iss),
    std::istream_iterator<std::string>(),
    std::back_inserter(tokens));
}

Step* step_parser::parse(const std::vector<std::string>& tokens) {
  Step* step = nullptr;
  parse_tokens(tokens, step);
  return step;
}
