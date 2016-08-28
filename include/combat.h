#pragma once

#include <cstdint>

struct CombatStats {
  CombatStats() 
    : m_action_points(3)
    , m_health(1.f)
    , m_attack(0.f)
    , m_range(0.f) {};

  CombatStats(uint32_t action_points, float health, float attack, float range) 
    : m_action_points(action_points)
    , m_health(health)
    , m_attack(attack)
    , m_range(range) {};

  uint32_t m_action_points;
  float m_health;
  float m_attack;
  float m_range;
};

struct Modifier {
  void reset() { 
    m_health_modifier = 1.0f;
    m_attack_modifier = 1.0f;
    m_range_modifier = 1.0f;
  };

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

  bool calculate_modifiers(Unit* attacker, Unit* defender, Modifier& attacker_modifier, Modifier& defender_modifier);
}
