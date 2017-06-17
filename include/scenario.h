#pragma once

#include <cstdint>

#include "enum_generated.h"

namespace scenario {
  void start(fbs::SCENARIO_TYPE type);
  void reset();
  void process();

  void debug_print();
}
