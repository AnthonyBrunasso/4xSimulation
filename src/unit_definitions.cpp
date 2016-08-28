#include "unit_definitions.h"

#include <cstdint>
#include <unordered_map>

#include "util.h"
#include "game_types.h"

namespace {
  typedef std::unordered_map<uint32_t, CombatStats> DefinitionMap;
  DefinitionMap s_definitions;
}

void unit_definitions::initialize() {
  // Scout
  s_definitions[util::enum_to_uint(UNIT_TYPE::SCOUT)] =
    CombatStats(3, 5.f, 1.f, 1.f);  // 5 health, 1 attack, 1 range.

  // Archer
  s_definitions[util::enum_to_uint(UNIT_TYPE::ARCHER)] =
    CombatStats(2, 10.f, 3.f, 2.f); // 10 health, 3 attack, 2 range.

  // Phalanx
  s_definitions[util::enum_to_uint(UNIT_TYPE::PHALANX)] =
    CombatStats(2, 13.f, 3.f, 1.f); // 13 health, 3 attack, 1 range.

  // Worker
  s_definitions[util::enum_to_uint(UNIT_TYPE::WORKER)] = 
    CombatStats(2, 5.f, 0.f, 0.f);  // 5 health, 0 attack, 0 range.
}

CombatStats* unit_definitions::get(UNIT_TYPE id) {
  uint32_t uint_id = util::enum_to_uint(id);
  if (s_definitions.find(uint_id) == s_definitions.end()) {
    return nullptr;
  }
  return &s_definitions[uint_id];
}

void unit_definitions::add(UNIT_TYPE id, const CombatStats& stats) {
  s_definitions[util::enum_to_uint(id)] = stats;
}

void unit_definitions::for_each_definition(
    std::function<void(UNIT_TYPE id, const CombatStats& stats)> operation) {
  for (auto definition : s_definitions) {
    operation(util::uint_to_enum<UNIT_TYPE>(definition.first), definition.second);
  }
}
