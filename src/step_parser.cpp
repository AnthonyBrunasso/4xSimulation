#include "step_parser.h"

#include "step.h"
#include "util.h"
#include "format.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace {

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
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::COLONIZE);
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

    else if (tokens[0] == "kill") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::KILL);
    }

    else if (tokens[0] == "move") {
      CREATE_GENERIC_STEP(5, tokens, step, COMMAND::MOVE);
    }

    else if (tokens[0] == "purchase") {
      CREATE_GENERIC_STEP(3, tokens, step, COMMAND::PURCHASE);
    }

    else if (tokens[0] == "sell") {
      CREATE_GENERIC_STEP(2, tokens, step, COMMAND::SELL);
    }

    else if (tokens[0] == "spawn") {
      CHECK_VALID(5, tokens);
      step = new SpawnStep(COMMAND::SPAWN);
      SpawnStep* spawn_step = static_cast<SpawnStep*>(step);
      spawn_step->m_uintId = std::stoul(tokens[1]);
      spawn_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
    }

    else {
      std::cout << "Unrecognized step: " << format::tokens(tokens) << std::endl;
    }
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::tokens(tokens) << std::endl;
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