#pragma once

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "game_types.h"

namespace simulation_interface {
  flatbuffers::FlatBufferBuilder& GetFBB();

  void copy_to_netbuffer(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step);

  void start_faceoff();

  void join_player(AI_TYPE type, const std::string& name);
}