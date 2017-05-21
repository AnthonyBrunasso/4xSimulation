#include "scenario.h"

#include "game_types.h"
#include "scenario_monster.h"
#include "scenario_arena.h"
#include "scenario_citylife.h"

#include <algorithm>
#include <vector>
#include <iostream>

namespace scenario {
  std::vector<SCENARIO_TYPE> s_running;
}

void scenario::start(SCENARIO_TYPE type) {
  std::cout << "Starting scenario: " << get_scenario_name(type) << std::endl;
  if (std::find(s_running.begin(), s_running.end(), type) != s_running.end()) return;
  switch(type) {
    case SCENARIO_TYPE::DISEASE:
    case SCENARIO_TYPE::MONSTER:
      scenario_monster::start();
      break;
    case SCENARIO_TYPE::ARENA:
      scenario_arena::start();
      break;
    case SCENARIO_TYPE::CITYLIFE:
      scenario_citylife::start();
    default:
      break;
  }

  s_running.push_back(type);
}

void scenario::reset() {
  for (auto t : s_running) {
      switch (t) {
      case SCENARIO_TYPE::DISEASE:
      case SCENARIO_TYPE::MONSTER:
          scenario_monster::reset();
          break;
      case SCENARIO_TYPE::ARENA:
          scenario_arena::reset();
      case SCENARIO_TYPE::CITYLIFE:
          scenario_citylife::reset();
      default:
          break;
      }
  }
  s_running.clear();
}

void scenario::process() {
  std::cout << "Running scenario logc." << std::endl;
  for (auto t : s_running) {
    switch(t) {
      case SCENARIO_TYPE::DISEASE:
      case SCENARIO_TYPE::MONSTER:
        scenario_monster::process();
        break;
      case SCENARIO_TYPE::ARENA:
        scenario_arena::process();
      case SCENARIO_TYPE::CITYLIFE:
        scenario_citylife::process();
      default:
        break;
    }
  }
}

void scenario::debug_print(SCENARIO_TYPE type) {
  for (auto t : s_running) {
    switch(t) {
      case SCENARIO_TYPE::DISEASE:
      case SCENARIO_TYPE::MONSTER:
        scenario_monster::debug_print();
        break;
      case SCENARIO_TYPE::ARENA:
      case SCENARIO_TYPE::CITYLIFE:
      default:
        break;
    }
  }
}
