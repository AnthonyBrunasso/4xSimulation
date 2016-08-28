#include "ai_unit_decisions.h"

#include "unit.h"
#include "util.h"
#include "random.h"
#include "network_types.h"
#include "world_map.h"
#include "format.h"
#include "ai_shared.h"

#include <iostream>

namespace unit_decisions {
  UnitWander s_wander;
  UnitFight s_fight;

  char s_unit_buffer[256];
}

void UnitWander::operator()(uint32_t unit_id) {
  ai_shared::wander(unit_id);
}

void UnitFight::operator()(uint32_t unit_id) {
  // Find nearest unit and attack it.
  Unit* u = unit::get_unit(unit_id);
  if (!u) return;

  auto attack = [](const Unit& unit) {
  };
}

UnitWander& unit_decisions::get_wander() {
  return s_wander;
}

UnitFight& unit_decisions::get_fight() {
  return s_fight;
}
