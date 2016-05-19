#include "combat.h"

#include "hex.h"

#include <cmath>

void combat::engage(CombatStats& attack_stats, 
  const Modifier& attack_modifier, 
  CombatStats& defend_stats, 
  const Modifier& defend_modifier,
  uint32_t distance) {

  float attacker_health = attack_stats.m_health * attack_modifier.m_health_modifier;
  const float attacker_attack = attack_stats.m_attack * attack_modifier.m_attack_modifier;
  const float attacker_range = attack_stats.m_range * attack_modifier.m_range_modifier;

  float defender_health = defend_stats.m_health * defend_modifier.m_health_modifier;
  const float defender_attack = defend_stats.m_attack * defend_modifier.m_attack_modifier;
  const float defender_range = defend_stats.m_range * defend_modifier.m_range_modifier;

  // If attacker can't reach defender exit
  if (attacker_range < static_cast<float>(distance)) {
    return;
  }
  defender_health -= attacker_attack;
  defend_stats.m_health = static_cast<uint32_t>(std::max(0.0f, roundf(defender_health)));

  // Only return fire if the defender can reach the attacker
  if (defender_range < static_cast<float>(distance)) {
    return;
  }
  attacker_health -= defender_attack;
  attack_stats.m_health = static_cast<uint32_t>(std::max(0.0f, roundf(attacker_health)));
}

void combat::engage(CombatStats& attack_stats, CombatStats& defend_stats, uint32_t distance) {
  Modifier modifier;

  modifier.m_health_modifier = 1.0f;
  modifier.m_attack_modifier = 1.0f;
  modifier.m_range_modifier = 1.0f;

  engage(attack_stats, modifier, defend_stats, modifier, distance);
}