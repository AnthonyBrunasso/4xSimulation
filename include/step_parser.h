#pragma once

#include <string>
#include <vector>

enum class NETWORK_TYPE;

namespace step_parser {
  // Create step based on string
  std::vector<std::string> split_to_tokens(const std::string& line);
  size_t parse(const std::vector<std::string>& tokens, bool& game_over, void* buffer, size_t buffer_len);
  std::string get_active_player();
  uint32_t get_active_player_id();
  void set_active_player_id(uint32_t);
}
