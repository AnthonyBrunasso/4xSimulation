#include "entity_types.h"

#include "util.h"

namespace {
  const char* entity_type_names [] = {
    "Scout",
    "Archer",
    "Phalanx",
  };
}

const char* get_entity_name(ENTITY_TYPE type) {
  if (type > ENTITY_TYPE::UNITS_LAST) {
    return "Unkown entity.";
  }
  return entity_type_names[util::enum_to_uint(type)]; 
}
