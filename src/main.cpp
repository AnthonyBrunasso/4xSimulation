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

std::fstream s_target_file;

int main(int , char* []) {
  game_random::set_seed(3);
  std::vector<Step*> steps;
  simulation::start();
  
  // Initialize queries
  terminal::initialize();
  s_target_file.open("last_run", std::ios::out);
  // Enter interactive mode
  std::string input;
  bool game_over = false;
  do
  {
    std::cout << std::endl;
    std::cout << step_parser::get_active_player() << " (turn " << simulation::get_turn() << ")> ";
    std::getline(std::cin, input);
    if (!std::cin.good()) {
      break;
    }

    std::vector<std::string> tokens = terminal::tokenize(input);

    if (terminal::is_query(tokens)) {
      terminal::run_query(tokens);
      continue;
    }

    if (terminal::run_step(tokens, game_over)) {
      // If a valid step and not a quit, save the command to file.
      if (!game_over) {
        s_target_file << input << std::endl;
      }
    }
  } while (!game_over);

  s_target_file.close();
  terminal::kill();
  simulation::kill();
  barbarians::destroy();

  return 0;
}
