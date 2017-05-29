#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "simulation.h"
#include "terminal.h"
#include "file_reader.h"
#include "ai_barbarians.h"
#include "step_parser.h"
#include "random.h"
#include "game_types.h"

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"

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

int main(int, char*[]) {
  game_random::set_seed(3);
  std::vector<Step*> steps;
  simulation::start();

  start_faceoff();

  // Enter interactive mode
  std::string input;
  std::getline(std::cin, input);

  simulation::kill();
  barbarians::reset();

  return 0;
}
