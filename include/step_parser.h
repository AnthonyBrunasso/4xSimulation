#pragma once

#include <string>
#include <vector>

struct Step;

namespace step_parser {
  // Create step based on string
  void split_to_tokens(const std::string& line, std::vector<std::string>& tokens);
  Step* parse(const std::vector<std::string>& tokens);
  std::string get_active_player();
  uint32_t get_active_player_id();
}
