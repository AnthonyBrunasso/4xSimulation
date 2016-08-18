#pragma once

enum class SCENARIO_TYPE;

namespace scenario {
  void start(SCENARIO_TYPE type);
  void process();
}
