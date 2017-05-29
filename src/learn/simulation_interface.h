#pragma once

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "game_types.h"

namespace simulation_interface {
  void start_faceoff();
  void join_player(AI_TYPE type, const std::string& name);
  void end_turn(uint32_t player_id, uint32_t next_player);
}