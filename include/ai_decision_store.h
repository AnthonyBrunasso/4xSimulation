#pragma once

#include "ai_decisions.h"

namespace decisions {
  Settle& get_settle();
  Explore& get_explore();
  Construct& get_construct();
  UnitDecision& get_unit();
}
