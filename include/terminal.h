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
    
  std::vector<std::string> tokenize(const std::string& input);
  bool is_query(const std::vector<std::string> &);
  bool run_query(const std::vector<std::string> &);
  bool run_step(const std::vector<std::string> &, bool& game_over);
}
