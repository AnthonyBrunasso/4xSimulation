#include "simulation_interface.h"

#include "step_generated.h"
#include "simulation.h"

#include <iostream>

namespace simulation_interface {
  const size_t BUFFER_LEN = 512;
  char buffer[BUFFER_LEN];

  flatbuffers::FlatBufferBuilder& GetFBB() {
    static flatbuffers::FlatBufferBuilder builder;
    return builder;
  }

  void copy_to_netbuffer(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step) {
    flatbuffers::Offset<fbs::AnyStep> anystep = fbs::CreateAnyStep(GetFBB(), step_type, step);
    fbs::FinishAnyStepBuffer(GetFBB(), anystep);
    if (GetFBB().GetSize() > BUFFER_LEN) {
      std::cout << "FlatBufferBuilder exceeded network buffer!! StepType: " << (uint32_t)step_type << std::endl;
      return;
    }

    std::memcpy(buffer, GetFBB().GetBufferPointer(), GetFBB().GetSize());
  }

  void start_faceoff() {
    SCENARIO_TYPE scenario_type = SCENARIO_TYPE::FACEOFF;
    flatbuffers::Offset<fbs::ScenarioStep> scenario_step = fbs::CreateScenarioStep(GetFBB(), (fbs::SCENARIO_TYPE)scenario_type);
    copy_to_netbuffer(fbs::StepUnion::ScenarioStep, scenario_step.Union());
    simulation::process_step(buffer, BUFFER_LEN);
  }

  void join_player(AI_TYPE type, const std::string& name) {
    AI_TYPE ai_type = type;
    flatbuffers::Offset<flatbuffers::String> fbs_name = GetFBB().CreateString(name);
    flatbuffers::Offset<fbs::AddPlayerStep> add_player_step = fbs::CreateAddPlayerStep(GetFBB(), fbs_name, (fbs::AI_TYPE)ai_type);
    copy_to_netbuffer(fbs::StepUnion::AddPlayerStep, add_player_step.Union());
    simulation::process_step(buffer, BUFFER_LEN);
  }
}