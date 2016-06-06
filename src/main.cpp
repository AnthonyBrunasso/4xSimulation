#include <iostream>
#include <vector>

#include "simulation.h"
#include "terminal.h"
#include "step.h"
#include "file_reader.h"

int main(int argc, char* argv[]) {
  std::vector<Step*> steps;
  simulation::start();

  // Process steps from file
  if (argc > 1) {
    file_reader::extract_steps(argv[1], steps);
    for (auto step : steps) {
      simulation::process_step(step);
      delete step;
    }
  }

  // Initialize queries
  terminal::initialize();
  terminal::output_steps("last_run");
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

  simulation::kill();

  return 0;
}
