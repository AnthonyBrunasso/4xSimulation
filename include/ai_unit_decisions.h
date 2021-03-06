#pragma once

#include <stdint.h>

#include "dtree.h"

class UnitWander : public Decision {
public:
  virtual void operator()(uint32_t unit_id) override;
};

class UnitFight : public Decision {
public:
  virtual void operator()(uint32_t unit_id) override;
};

namespace unit_decisions {
  UnitWander& get_wander();
  UnitFight& get_fight();
}
