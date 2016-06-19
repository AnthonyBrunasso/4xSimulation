#include "ai_state.h"

#include <iostream>

AIState::AIState() {

}

void AIState::add_order(uint32_t unit_id, uint32_t target_id, AI_ORDER_TYPE order) {
  std::cout << "Issuing order: " << get_ai_order_name(order) << " unit_id: " << unit_id << " target_id: " << target_id << std::endl;
  m_orders.push_back(UnitOrder(unit_id, target_id, order));
}
