#pragma once

#include <stddef.h>
#include <functional>
#include <string>
#include <vector>

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
  size_t step_to_bytes(const std::vector<std::string>& tokens, void* buffer, size_t buffer_len);
}
