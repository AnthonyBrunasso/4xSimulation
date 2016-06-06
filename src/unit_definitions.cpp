#include "unit_definitions.h"

#include <cstdint>
#include <unordered_map>

#include "util.h"

namespace {
  typedef std::unordered_map<uint32_t, CombatStats> DefinitionMap;
  DefinitionMap s_definitions;
}

void unit_definitions::initialize() {
  // Scout
  s_definitions[util::enum_to_uint(ENTITY_TYPE::SCOUT)] =
    CombatStats(5, 1, 1); // 5 health, 1 attack, 1 range.

  // Archer
  s_definitions[util::enum_to_uint(ENTITY_TYPE::ARCHER)] =
    CombatStats(10, 3, 2); // 10 health, 3 attack, 2 range.

  // Phalanx
  s_definitions[util::enum_to_uint(ENTITY_TYPE::PHALANX)] =
    CombatStats(13, 3, 1); // 13 health, 3 attack, 1 range.
}

CombatStats* unit_definitions::get(ENTITY_TYPE id) {
  uint32_t uint_id = util::enum_to_uint(id);
  if (s_definitions.find(uint_id) == s_definitions.end()) {
    return nullptr;
  }
  return &s_definitions[uint_id];
}

void unit_definitions::add(ENTITY_TYPE id, const CombatStats& stats) {
  s_definitions[util::enum_to_uint(id)] = stats;
}

void unit_definitions::for_each_definition(
    std::function<void(ENTITY_TYPE id, const CombatStats& stats)> operation) {
  for (auto definition : s_definitions) {
    operation(util::uint_to_enum<ENTITY_TYPE>(definition.first), definition.second);
  }
}
