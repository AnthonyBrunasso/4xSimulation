#include <iostream>
#include <vector>

#include "simulation.h"
#include "terminal.h"
#include "step.h"
#include "file_reader.h"

int main(int , char* []) {
  std::vector<Step*> steps;
  simulation::start();
  
  // Initialize queries
  terminal::initialize();
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

  return 0;
}
