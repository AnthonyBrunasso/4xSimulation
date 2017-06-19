#pragma once

#include <cstdint>

#include "enum_generated.h"

// Resources represent uncommon terrain properties
// Such as Gold, Iron, or Deer
struct Resource {
  fbs::RESOURCE_TYPE m_type;
  int32_t m_quantity;
};

