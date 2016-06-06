#include <iostream>
#include <vector>

#include "simulation.h"
#include "terminal.h"
#include "step.h"
#include "file_reader.h"

int main(int argc, char* argv[]) {
  std::vector<Step*> steps;
  simulation::start();
  
  std::vector<std::string>* steps_strings = nullptr;
  // Process steps from file
  if (argc > 1) {
    steps_strings = new std::vector<std::string>();
    file_reader::extract_steps(argv[1], steps, steps_strings);
    for (auto step : steps) {
      simulation::process_step(step);
      delete step;
    }
  }

  terminal::output_steps("last_run", steps_strings);
  if (steps_strings) {
    delete steps_strings;
  }

  // Initialize queries
  terminal::initialize();
  // Enter interactive mode
  while (Step* step = terminal::parse_input()) {
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
