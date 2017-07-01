#include "ai_unit_decisions.h"

#include "ai_shared.h"
#include "search.h"
#include "unit.h"

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
  
  uint32_t found_id = 0;; 
  auto found = [&found_id, &u](const Unit& unit) {
    if (u->m_owner_id != unit.m_owner_id) {
      found_id = unit.m_id; 
      return true;
    }
    return false;
  };

  search::bfs_units(u->m_location, 4, found);

  if (found_id) {
    // Awkwardly this will try to attack as well.
    if (ai_shared::approach_unit(unit_id, found_id)) return;
  }

  // Otherwise just wander
  unit_decisions::get_wander()(unit_id);
}

UnitWander& unit_decisions::get_wander() {
  return s_wander;
}

UnitFight& unit_decisions::get_fight() {
  return s_fight;
}
