#include "unit_definitions.h"

#include <cstdint>
#include <unordered_map>
#include <utility>

#include "combat.h"

#include "step_generated.h"
#include "util.h"

namespace {
  constexpr size_t MAX_UNITS = static_cast<size_t>(fbs::UNIT_TYPE::MAX);
  CombatStats s_array[MAX_UNITS];
}

void unit_definitions::initialize() {
  // CombatStats: movement, health, attack, backstab, range

  // Scout
  add(any_enum(fbs::UNIT_TYPE::SCOUT),
    CombatStats(3, 19.f, 11.f, 19.f, 1.f)); 

  // Archer
  add(any_enum(fbs::UNIT_TYPE::ARCHER),
    CombatStats(2, 26.f, 13.f, 23.f, 2.f));

  // Phalanx
  add(any_enum(fbs::UNIT_TYPE::PHALANX),
    CombatStats(2, 47.f, 17.f, 37.f, 1.f));

  // Worker
  add(any_enum(fbs::UNIT_TYPE::WORKER),
    CombatStats(2, 38.f, 0.f, 0.f, 0.f));

  // Wizard
  add(any_enum(fbs::UNIT_TYPE::WIZARD),
    CombatStats(2, 18.f, 7.f, 7.f, 1.f));
}

void unit_definitions::reset() {
  for (CombatStats& cs : s_array) {
    cs = CombatStats();
  }
}

CombatStats* unit_definitions::get(fbs::UNIT_TYPE id) {
  uint32_t uint_id = any_enum(id);
  if (uint_id >= MAX_UNITS) return nullptr;

  return &s_array[uint_id];
}

void unit_definitions::add(fbs::UNIT_TYPE id, const CombatStats& stats) {
  uint32_t uint_id = any_enum(id);
  if (uint_id >= MAX_UNITS) return;

  s_array[uint_id] = stats;
}

void unit_definitions::for_each_definition(
    std::function<void(fbs::UNIT_TYPE id, const CombatStats& stats)> operation) {
  for (auto ut : fbs::EnumValuesUNIT_TYPE()) {
    operation(ut, s_array[any_enum(ut)]);
  }
}

