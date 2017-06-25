
#pragma once

#include <cstdint>

#include "enum_generated.h"

class ConstructionOrder
{
public:
  explicit ConstructionOrder()
    : m_type(fbs::CONSTRUCTION_TYPE::UNKNOWN)
    , m_city_id(0)
    , m_production(0.0)
  {}

  fbs::CONSTRUCTION_TYPE m_type;
  uint32_t m_city_id;
  float m_production;
  uint32_t m_id;
};

