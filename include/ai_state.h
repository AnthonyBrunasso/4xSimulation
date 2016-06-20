#pragma once

#include "game_types.h"

#include <vector>

// Order to be issued to unit.
class UnitOrder {
public:
  UnitOrder() : 
    m_unit_id(0)
    , m_target_id(0)
    , m_order(AI_ORDER_TYPE::UNKNOWN) {};

  UnitOrder(uint32_t unit_id, uint32_t target_id, AI_ORDER_TYPE order) :
    m_unit_id(unit_id)
    , m_target_id(target_id)
    , m_order(order) {};

  uint32_t m_unit_id;
  uint32_t m_target_id;
  AI_ORDER_TYPE m_order;
};

class AIState {
public:
  AIState();

  void add_order(uint32_t unit_id, uint32_t target_id, AI_ORDER_TYPE order);

  std::vector<UnitOrder> m_orders;
};