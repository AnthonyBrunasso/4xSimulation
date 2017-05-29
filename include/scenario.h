#pragma once

#include <cstdint>

namespace fbs
{
  enum class SCENARIO_TYPE : uint32_t;
}

namespace scenario {
  void start(fbs::SCENARIO_TYPE type);
  void reset();
  void process();

  void debug_print(fbs::SCENARIO_TYPE type);
}
