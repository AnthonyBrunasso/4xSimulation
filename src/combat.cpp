#include "combat.h"

#include "custom_math.h"
#include "hex.h"
#include "unit.h"

#include <cmath>
#include <algorithm>
#include <iostream>

// Engaging returns true if combat occurred, false otherwise
bool combat::engage(
  Unit* attacker,
  Unit* defender) {
  CombatStats& attack_stats = attacker->m_combat_stats;
  CombatStats& defend_stats = defender->m_combat_stats;

  // Attacker faces defender on combat initiation.
  unit::change_direction(attacker->m_id, defender->m_location);

  // Get distance between characters
  uint32_t distance = hex::cube_distance(attacker->m_location, defender->m_location);
  bool is_backstab = cmath::dot(attacker->m_direction, defender->m_direction) > 0;

  float attacker_health = attack_stats.m_health;
  float attacker_attack = attack_stats.m_attack;
  const float attacker_range = attack_stats.m_range;
  if (is_backstab) {
    std::cout << "Attack is a BACKSTAB!" << std::endl;
    attacker_attack = attack_stats.m_backstab;
  }

  float defender_health = defend_stats.m_health;
  const float defender_attack = defend_stats.m_attack;
  const float defender_range = defend_stats.m_range;

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

  // Defender turns to face attacker only if he engaged in combat.
  unit::change_direction(defender->m_id, attacker->m_location);

  return true;
}
