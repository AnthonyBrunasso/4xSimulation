
#pragma once

class ConstructionOrder
{
public:
  explicit ConstructionOrder(CONSTRUCTION_TYPE type_id, uint32_t city_id)
    : m_type(type_id)
    , m_city_id(city_id)
    , m_production(0.0)
  {}

  CONSTRUCTION_TYPE m_type;
  uint32_t m_city_id;
  float m_production;
};
