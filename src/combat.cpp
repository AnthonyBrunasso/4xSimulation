#include "combat.h"

#include "hex.h"
#include "units.h"
#include "custom_math.h"

#include <cmath>
#include <algorithm>
#include <iostream>

// Engaging returns true if combat occurred, false otherwise
bool combat::engage(CombatStats& attack_stats, 
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
    std::cout << "Attack can't reach defender" << std::endl;
    return false;
  }

  defender_health -= attacker_attack;
  defend_stats.m_health = (std::max(0.0f, roundf(defender_health)));
  std::cout << "Defender is dealt " << attacker_attack << " damage! " << std::endl;
  std::cout << "Defender health is " << defend_stats.m_health << std::endl;

  // Only return fire if the defender can reach the attacker
  if (defender_range < static_cast<float>(distance)) {
    std::cout << "Defender can't reach attacker" << std::endl;
    return true;
  }

  attacker_health -= defender_attack;
  attack_stats.m_health = (std::max(0.0f, roundf(attacker_health)));
  std::cout << "Attacker received " << defender_attack << " damage! " << std::endl;
  std::cout << "Attacker health is " << attack_stats.m_health << std::endl;

  return true;
}

bool combat::engage(CombatStats& attack_stats, CombatStats& defend_stats, uint32_t distance) {
  Modifier modifier;

  modifier.m_health_modifier = 1.0f;
  modifier.m_attack_modifier = 1.0f;
  modifier.m_range_modifier = 1.0f;

  return engage(attack_stats, modifier, defend_stats, modifier, distance);
}

bool combat::calculate_modifiers(Unit* attacker, Unit* defender, Modifier& attacker_modifier, Modifier& defender_modifier) {
  if (!attacker || !defender) return false;

  attacker_modifier.reset();
  defender_modifier.reset();

  // Attacker and defender are facing *nearly* the same direction, this is a backstab.
  if (cmath::dot(attacker->m_direction, defender->m_direction) > 0) {
    std::cout << attacker->m_unique_id << " backstabs " << defender->m_unique_id << std::endl;
    attacker_modifier.m_attack_modifier = 1.5f;
  }

  return true;
}
