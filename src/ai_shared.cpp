#include "ai_shared.h"

#include <iostream>

#include "unit.h"
#include "city.h"
#include "improvement.h"
#include "random.h"
#include "world_map.h"
#include "format.h"

#include "step_generated.h"

namespace ai_shared {
  const size_t BUFFER_LEN = 256;
  char s_ai_buffer[BUFFER_LEN];

  flatbuffers::FlatBufferBuilder& GetFBB() {
    static flatbuffers::FlatBufferBuilder builder;
    return builder;
  }
}

void ai_shared::simulate_step(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step)
{
  flatbuffers::Offset<fbs::AnyStep> anystep = fbs::CreateAnyStep(GetFBB(), step_type, step);
  fbs::FinishAnyStepBuffer(GetFBB(), anystep);
  simulation::process_step_from_ai(GetFBB().GetBufferPointer(), GetFBB().GetSize());
  GetFBB().Clear();
}

bool ai_shared::attack_unit(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Unit* tu = unit::get_unit(target_id);
  if (!su || !tu) {
    if (!tu) std::cout << "Target is gone id: " << target_id << std::endl;
    return false; 
  }

  // Don't attempt an attack if out of action points.
  if (!su->m_action_points) {
    std::cout << "No action points to attack" << std::endl;
    // Return true, decision is a success, just out of action points.
    return true;
  }
  
  flatbuffers::Offset<fbs::AttackStep> attack_step;
  uint32_t attacker_id = (unit_id);
  uint32_t defender_id = (target_id);
  uint32_t player_id = (su->m_owner_id);
  attack_step = fbs::CreateAttackStep(GetFBB(), attacker_id, defender_id, player_id);
  simulate_step(fbs::StepUnion::AttackStep, attack_step.Union());
  return true;
}

bool ai_shared::attack_city(uint32_t unit_id, uint32_t city_id)
{
  Unit* u = unit::get_unit(unit_id);
  if (!u) return false;

  flatbuffers::Offset<fbs::SiegeStep> siege_step;
  uint32_t player_id = (u->m_owner_id);
  siege_step = fbs::CreateSiegeStep(GetFBB(), player_id, unit_id, city_id);
  simulate_step(fbs::StepUnion::SiegeStep, siege_step.Union());
  return true;
}

void ai_shared::approach(uint32_t unit_id, const sf::Vector3i& location) {
  Unit* u = unit::get_unit(unit_id);
  if (!u) return;
  flatbuffers::Offset<fbs::MoveStep> move_step;
  uint32_t player_id = (u->m_owner_id);
  fbs::v3i dest(location.x, location.y, location.z);
  bool immediate = (true);
  bool avoid_city = (true);
  bool avoid_unit = (true);
  bool require_ownership = (true);
  move_step = fbs::CreateMoveStep(GetFBB(), unit_id, &dest, player_id, immediate, avoid_unit, avoid_city, require_ownership);
  simulate_step(fbs::StepUnion::MoveStep, move_step.Union());
}

bool ai_shared::approach_unit(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Unit* tu = unit::get_unit(target_id);
  if (!su || !tu) {
    if (!tu) std::cout << "Target unit is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, tu->m_location);
  // Try to attack the unit.
  attack_unit(unit_id, target_id);
  return true;
}

bool ai_shared::approach_city(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  City* tc = city::get_city(target_id);
  if (!su || !tc) {
    if (!tc) std::cout << "Target city is gone id: " << target_id << std::endl;
    return false; 
  }

  approach(unit_id, tc->m_location);
  return true;
}

bool ai_shared::pillage_improvement(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Improvement* ti = improvement::get_improvement(target_id);
  if (!su || !ti) {
    if (!ti) std::cout << "Target improvement is gone id: " << target_id << std::endl;
    return false; 
  }

  // Don't attempt an attack if out of action points.
  if (!su->m_action_points) {
    // Return true, decision is a success, just out of action points.
    return true;
  }
 
  flatbuffers::Offset<fbs::PillageStep> pillage_step;
  
  uint32_t player_id = (su->m_owner_id);
  pillage_step = fbs::CreatePillageStep(GetFBB(), player_id, unit_id);
  simulate_step(fbs::StepUnion::PillageStep, pillage_step.Union());
  return true;
}

bool ai_shared::wander(uint32_t unit_id) {
  Unit* su = unit::get_unit(unit_id);
  if (!su) {
    return false;
  }

  // Move to the improvement.
  flatbuffers::Offset<fbs::MoveStep> move_step;
  sf::Vector3i random(get_random_coord());
  fbs::v3i dest(random.x, random.y, random.z);
  uint32_t player_id = (su->m_owner_id);
  bool immediate = (true);
  bool avoid_city = (true);
  bool avoid_unit = (true);
  bool require_ownership = (true);
  move_step = fbs::CreateMoveStep(GetFBB(), unit_id, &dest, player_id, immediate, avoid_unit, avoid_city, require_ownership);
  simulate_step(fbs::StepUnion::MoveStep, move_step.Union());

  return true;
}

bool ai_shared::approach_improvement(uint32_t unit_id, uint32_t target_id) {
  Unit* su = unit::get_unit(unit_id);
  Improvement* ti = improvement::get_improvement(target_id);
  if (!su || !ti) {
    if (!ti) std::cout << "Target improvement is gone id: " << target_id << std::endl;
    return false; 
  } 

  // Move to the improvement.
  flatbuffers::Offset<fbs::MoveStep> move_step;
  fbs::v3i dest(ti->m_location.x, ti->m_location.y, ti->m_location.z);
  uint32_t player_id = (su->m_owner_id);
  bool immediate = (true);
  bool avoid_city = (true);
  bool avoid_unit = (true);
  bool require_ownership = (true);
  move_step = fbs::CreateMoveStep(GetFBB(), unit_id, &dest, player_id, immediate, avoid_unit, avoid_city, require_ownership);
  simulate_step(fbs::StepUnion::MoveStep, move_step.Union());
  // Try to pillage it.
  pillage_improvement(unit_id, target_id);
  return true;
}

sf::Vector3i ai_shared::get_random_coord() {
  sf::Vector3i coord = game_random::cube_coord(6);
  std::cout << format::vector3(coord) << std::endl;
  Tile* tile = world_map::get_tile(coord);
  while (!tile) {
    coord = game_random::cube_coord(6);
    tile = world_map::get_tile(coord); 
  }
  return coord;
}



