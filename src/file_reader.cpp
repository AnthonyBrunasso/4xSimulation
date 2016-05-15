#include "file_reader.h"

#include "step.h"
#include "step_parser.h"

#include <iostream>
#include <fstream>

void file_reader::extract_steps(const std::string& filename, std::vector<Step*>& steps) {
  std::ifstream input(filename);
  std::string line;
  std::vector<std::string> tokens;

  while (std::getline(input, line)) {
    // Previous tokens are cleared on each call to this function
    step_parser::split_to_tokens(line, tokens);
    Step* step = step_parser::parse(tokens);
    if (!step) continue;
    steps.push_back(step);
  }
}