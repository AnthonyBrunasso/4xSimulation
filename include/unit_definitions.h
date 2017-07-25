#pragma once

#include <cstdint>
#include <cstring>
#include <functional>

class CombatStats {
public:
  CombatStats()
  {
    memset(this, 0, sizeof(CombatStats));
  }

  CombatStats(uint32_t action_points, float health, float attack, float backstab, float range, int food)
    : m_action_points(action_points)
    , m_health(health)
    , m_attack(attack)
    , m_backstab(backstab)
    , m_range(range)
    , m_food(food) {};

  uint32_t m_action_points;
  float m_health;
  float m_attack;
  float m_backstab;
  float m_range;
  int m_food;

private:
};

namespace fbs {
  enum class UNIT_TYPE : uint32_t;
}

// Unit definitions will contain some initial stats for certain types of unit.
namespace unit_definitions {
  // This function will contain prebuilt unit definitions.
  void initialize();
  void reset();
  CombatStats* get(fbs::UNIT_TYPE id);
  void add(fbs::UNIT_TYPE id, const CombatStats& stats);

  void for_each_definition(
      std::function<void(fbs::UNIT_TYPE type, const CombatStats& stats)> operation);
}
