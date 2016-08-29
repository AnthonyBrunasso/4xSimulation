#pragma once

#include <cstdint>

class CombatStats {
public:
  CombatStats()
    : m_action_points(3)
    , m_health(1.f)
    , m_attack(0.f)
    , m_backstab(0.f)
    , m_range(0.f) {};

  CombatStats(uint32_t action_points, float health, float attack, float backstab, float range) 
    : m_action_points(action_points)
    , m_health(health)
    , m_attack(attack)
    , m_backstab(backstab)
    , m_range(range) {};

  uint32_t m_action_points;
  float m_health;
  float m_attack;
  float m_backstab;
  float m_range;
};

class Unit;

namespace combat {
  bool engage(
    Unit* attacker,
    Unit* defender);
}
