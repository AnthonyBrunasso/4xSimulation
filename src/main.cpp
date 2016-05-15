#include <iostream>
#include <vector>

#include "simulation.h"

void read_file(std::string filename, std::vector<Step*>& steps) {

}

int main() {
  std::vector<Step*> steps;
  const std::string filename = "";

  read_file(filename, steps);

  for (auto step : steps) {
    simulation::process_step(step);
  }

  return 0;
}
