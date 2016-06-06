#pragma once

// Ids that identify the type of a unit, building, etc

enum class ENTITY_TYPE {
  // Bros start at 0
  SCOUT = 0,
  ARCHER,
  PHALANX,
  UNITS_FIRST = SCOUT,
  UNITS_LAST = PHALANX,
  // Buildings start at 1000
  TOWN = 1000,
};

const char* get_entity_name(ENTITY_TYPE type);
