#include <iostream>
#include <vector>
#include <string>

#include "simulation.h"
#include "terminal.h"
#include "file_reader.h"
#include "ai_barbarians.h"
#include "step_parser.h"
#include "random.h"

int main(int , char* []) {
  game_random::set_seed(3);
  std::vector<Step*> steps;
  simulation::start();
  
  // Initialize queries
  terminal::initialize();
  terminal::record_steps("last_run");
  // Enter interactive mode
  std::string value;
  do
  {
    std::cout << std::endl;
    std::cout << step_parser::get_active_player() << " (turn " << simulation::get_turn() << ")> ";
    std::getline(std::cin, value);
    if (!std::cin.good()) {
      break;
    }
  } while (terminal::parse_input(value));

  terminal::kill();
  simulation::kill();
  barbarians::destroy();

  return 0;
}
