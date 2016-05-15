#include <iostream>
#include <vector>

#include "simulation.h"
#include "terminal.h"
#include "step.h"

void read_file(std::string filename, std::vector<Step*>& steps) {

}

int main(int argc, char* argv[]) {
  std::vector<Step*> steps;
  const std::string filename = "";

  simulation::start();

  // Process steps from file
  if (argc > 1) {
    read_file(filename, steps);

    for (auto step : steps) {
      simulation::process_step(step);
      delete step;
    }
  }

  // Enter interactive mode
  while (Step* step = terminal::parse_input()) {
    simulation::process_step(step);
    if (step->m_command == COMMAND::QUIT) {
      delete step;
      break;
    }
    // Finished with the step, clean it up
    delete step;
  }

  return 0;
}
