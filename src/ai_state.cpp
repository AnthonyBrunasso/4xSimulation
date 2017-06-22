#include "ai_state.h"

#include <iostream>

UnitOrder::UnitOrder() 
  : m_unit_id(0)
  , m_target_id(0)
  , m_order(fbs::AI_ORDER_TYPE::UNKNOWN)
{}

AIState::AIState()
: m_micro_done(false)
{

}

void AIState::add_order(uint32_t unit_id, uint32_t target_id, fbs::AI_ORDER_TYPE order) {
  std::cout << "Issuing order: " << fbs::EnumNameAI_ORDER_TYPE(order) << " unit_id: " << unit_id << " target_id: " << target_id << std::endl;
  m_orders.push_back(UnitOrder(unit_id, target_id, order));
}
