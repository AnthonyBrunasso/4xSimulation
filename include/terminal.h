#pragma once

#include <functional>
#include <vector>
#include <string>

namespace terminal {
  void initialize();
  void kill();
  void add_query(
    const std::string& command, 
    const std::string& help,
    std::function<bool(const std::vector<std::string>&)> operation);
    
  bool parse_input(const std::string& input);
  void record_steps(const std::string& filename);
}
