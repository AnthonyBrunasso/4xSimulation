#include "scenario.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "scenario_arena.h"
#include "scenario_citylife.h"
#include "scenario_faceoff.h"
#include "scenario_monster.h"
#include "util.h"

namespace scenario {
  std::vector<uint32_t> s_running;
}

void scenario::start(fbs::SCENARIO_TYPE scenario) {
  std::cout << "Starting scenario: " << fbs::EnumNameSCENARIO_TYPE(scenario) << std::endl;

  uint32_t scenario_val = any_enum(scenario);
  if (std::find(s_running.begin(), s_running.end(), scenario_val) != s_running.end()) return;

  switch(scenario) {
    case fbs::SCENARIO_TYPE::DISEASE:
    case fbs::SCENARIO_TYPE::MONSTER:
      scenario_monster::start();
      break;
    case fbs::SCENARIO_TYPE::FACEOFF:
      scenario_faceoff::start();
      break;
    case fbs::SCENARIO_TYPE::ARENA:
      scenario_arena::start();
      break;
    case fbs::SCENARIO_TYPE::CITYLIFE:
      scenario_citylife::start();
      break;
    default:
      break;
  }

  s_running.push_back(scenario_val);
}

void scenario::reset() {
  scenario_monster::reset();
  scenario_faceoff::reset();
  scenario_arena::reset();
  scenario_citylife::reset();

  s_running.clear();
}

void scenario::process() {
  std::cout << "Running scenario logc." << std::endl;
  for (auto t : s_running) {
    switch(fbs::SCENARIO_TYPE(t)) {
      case fbs::SCENARIO_TYPE::DISEASE:
      case fbs::SCENARIO_TYPE::MONSTER:
        scenario_monster::process();
        break;
      case fbs::SCENARIO_TYPE::ARENA:
        scenario_arena::process();
      case fbs::SCENARIO_TYPE::CITYLIFE:
        scenario_citylife::process();
      default:
        break;
    }
  }
}

void scenario::debug_print() {
  for (auto t : s_running) {
    switch(fbs::SCENARIO_TYPE(t)) {
      case fbs::SCENARIO_TYPE::DISEASE:
      case fbs::SCENARIO_TYPE::MONSTER:
        scenario_monster::debug_print();
        break;
      case fbs::SCENARIO_TYPE::FACEOFF:
      case fbs::SCENARIO_TYPE::ARENA:
      case fbs::SCENARIO_TYPE::CITYLIFE:
      default:
        break;
    }
  }
}
