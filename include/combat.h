#pragma once

#include <cstdint>

struct CombatStats {
  CombatStats() :
    m_health(1.f)
    , m_attack(0.f)
    , m_range(0.f) {};

  CombatStats(float health, float attack, float range) :
    m_health(health)
    , m_attack(attack)
    , m_range(range) {};

  float m_health;
  float m_attack;
  float m_range;
};

struct Modifier {
  float m_health_modifier;
  float m_attack_modifier;
  float m_range_modifier;
};

class Unit;

namespace combat {
  bool engage(CombatStats& attack_stats, 
    const Modifier& attack_modifier, 
    CombatStats& defend_stats, 
    const Modifier& defend_modifer,
    uint32_t distance);

  // Calls above function with all modifers equal to 1.0
  bool engage(CombatStats& attack_stats, CombatStats& defend_stats, uint32_t distance);
}
