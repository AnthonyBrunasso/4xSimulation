#pragma once

#include <cstdint>

struct CombatStats {
  CombatStats(uint32_t health, uint32_t attack, uint32_t range) :
    m_health(health)
    , m_attack(attack)
    , m_range(range) {};

  uint32_t m_health;
  uint32_t m_attack;
  uint32_t m_range;
};

struct Modifier {
  float m_health_modifier;
  float m_attack_modifier;
  float m_range_modifier;
};

class Unit;

namespace combat {
  void engage(CombatStats& attack_stats, 
    const Modifier& attack_modifier, 
    CombatStats& defend_stats, 
    const Modifier& defend_modifer,
    uint32_t distance);

  // Calls above function with all modifers equal to 1.0
  void engage(CombatStats& attack_stats, CombatStats& defend_stats, uint32_t distance);
}