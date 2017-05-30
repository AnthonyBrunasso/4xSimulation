#include "unit_definitions.h"

#include <cstdint>
#include <unordered_map>
#include <utility>

#include "combat.h"

#include "step_generated.h"
#include "util.h"

namespace {
  typedef std::unordered_map<uint32_t, CombatStats> DefinitionMap;
  DefinitionMap s_definitions;
}

void unit_definitions::initialize() {
  // CombatStats: movement, health, attack, backstab, range

  // Scout
  s_definitions[util::enum_to_uint(fbs::UNIT_TYPE::SCOUT)] =
    CombatStats(3, 19.f, 11.f, 19.f, 1.f); 

  // Archer
  s_definitions[util::enum_to_uint(fbs::UNIT_TYPE::ARCHER)] =
    CombatStats(2, 26.f, 13.f, 23.f, 2.f);

  // Phalanx
  s_definitions[util::enum_to_uint(fbs::UNIT_TYPE::PHALANX)] =
    CombatStats(2, 48.f, 17.f, 37.f, 1.f);

  // Worker
  s_definitions[util::enum_to_uint(fbs::UNIT_TYPE::WORKER)] = 
    CombatStats(2, 38.f, 0.f, 0.f, 0.f);

  // Wizard
  s_definitions[util::enum_to_uint(fbs::UNIT_TYPE::WIZARD)] =
    CombatStats(2, 18.f, 7.f, 7.f, 1.f);
}

void unit_definitions::reset() {
  s_definitions.clear();
}

CombatStats* unit_definitions::get(fbs::UNIT_TYPE id) {
  uint32_t uint_id = util::enum_to_uint(id);
  if (s_definitions.find(uint_id) == s_definitions.end()) {
    return nullptr;
  }
  return &s_definitions[uint_id];
}

void unit_definitions::add(fbs::UNIT_TYPE id, const CombatStats& stats) {
  s_definitions[util::enum_to_uint(id)] = stats;
}

void unit_definitions::for_each_definition(
    std::function<void(fbs::UNIT_TYPE id, const CombatStats& stats)> operation) {
  for (auto definition : s_definitions) {
    operation(util::uint_to_enum<fbs::UNIT_TYPE>(definition.first), definition.second);
  }
}
