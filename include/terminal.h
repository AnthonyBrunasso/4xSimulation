#pragma once

#include <functional>
#include <vector>
#include <string>

struct Step;

namespace terminal {
  void add_query(
    const std::string& command, 
    const std::string& help,
    std::function<bool(const std::vector<std::string>&)> operation);

  Step* parse_input();
}