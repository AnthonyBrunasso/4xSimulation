#pragma once

#include <string>
#include <vector>

struct Step;

namespace file_reader {
  void extract_steps(const std::string& filename, std::vector<Step*>& steps);
}