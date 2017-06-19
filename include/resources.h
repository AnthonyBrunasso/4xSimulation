#pragma once

#include <cstdint>

#include "enum_generated.h"



// These resources will represent civilization level resources. 
// Examples: Amount of gold in a civilization, amount of happiness.
// Or a strategic resource:
// Amount of sugar or stone, for example.
struct Resource {
  explicit Resource();
  explicit Resource(fbs::RESOURCE_TYPE type);
  explicit Resource(fbs::RESOURCE_TYPE type, uint32_t quantity);

  bool operator==(const Resource& rhs) const;
  Resource& operator+=(const Resource& rhs);

  // Idicates the type of resource this is.
  fbs::RESOURCE_TYPE m_type;
  int32_t m_quantity;
};

