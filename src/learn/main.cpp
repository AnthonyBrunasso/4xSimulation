#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "simulation.h"
#include "terminal.h"
#include "file_reader.h"
#include "ai_barbarians.h"
#include "step_parser.h"
#include "random.h"
#include "game_types.h"
#include "player.h"

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "simulation_interface.h"

int main(int, char*[]) {
  game_random::set_seed(3);
  std::vector<Step*> steps;
  simulation::start();

  simulation_interface::join_player(AI_TYPE::BARBARIAN, "barbarian");
  simulation_interface::join_player(AI_TYPE::HUMAN, "learner");

  barbarians::set_player_id(0);

  simulation_interface::start_faceoff();

  // Enter interactive mode
  std::string input;
  std::getline(std::cin, input);

  simulation::kill();
  barbarians::reset();

  return 0;
}
