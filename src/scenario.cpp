#include "scenario.h"

#include "game_types.h"
#include "scenario_monster.h"
#include "scenario_arena.h"
#include "scenario_citylife.h"
#include "scenario_faceoff.h"
#include "step_generated.h"

#include <algorithm>
#include <vector>
#include <iostream>

namespace scenario {
  std::vector<fbs::SCENARIO_TYPE> s_running;
}

void scenario::start(fbs::SCENARIO_TYPE type) {
  std::cout << "Starting scenario: " << fbs::EnumNameSCENARIO_TYPE(type) << std::endl;
  if (std::find(s_running.begin(), s_running.end(), type) != s_running.end()) return;
  switch(type) {
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

  s_running.push_back(type);
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
    switch(t) {
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

void scenario::debug_print(fbs::SCENARIO_TYPE type) {
  for (auto t : s_running) {
    switch(t) {
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
