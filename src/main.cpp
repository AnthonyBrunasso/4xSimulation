#include <iostream>
#include <vector>

#include "simulation.h"
#include "terminal.h"
#include "step.h"
#include "file_reader.h"
#include "ai_barbarians.h"
#include "random.h"
#include <random>

int main(int , char* []) {
  std::mt19937 foo;
  foo.seed(3);
  
  game_random::set_seed(0);
  for (int i = 0; i < 5; ++i) {
    std::cout << foo() << std::endl;
  }
  std::vector<Step*> steps;
  simulation::start();
  
  // Initialize queries
  terminal::initialize();
  terminal::record_steps("last_run");
  // Enter interactive mode
  while (std::cin.good()) {
    Step* step = terminal::parse_input();
    if (!step) {
      break;
    }
    
    simulation::process_step(step);

    bool quitting = step->m_command == COMMAND::QUIT;
    // Finished with the step, clean it up
    delete step;

    if (quitting) {
      break;
    }
  }

  terminal::kill();
  simulation::kill();
  barbarians::destroy();

  return 0;
}
