#include "scenario_arena.h"
#include "player.h"
#include "city.h"
#include "unit.h"
#include <unordered_map>

namespace scenario_arena
{
  typedef std::unordered_map<uint32_t, uint32_t> PlayerScore;
  PlayerScore s_player_score;
  bool s_active;

  uint32_t score(UNIT_TYPE ut);
  void dead_unit(UnitFatality* uf);
};

uint32_t scenario_arena::score(UNIT_TYPE ut)
{
  switch(ut)
  {
  case UNIT_TYPE::MONSTER:
    return 5;
  default:
    break;
  }

  return 1;
}

void scenario_arena::dead_unit(UnitFatality* uf)
{
  // Don't score units that didn't have an opposing player
  if (!uf->m_opponent) return;

  s_player_score[uf->m_opponent->m_id] += score(uf->m_dead->m_type);

  if ((s_player_score[uf->m_opponent->m_id] & (4-1)) == 0) {
    unit::create(UNIT_TYPE::ARCHER, uf->m_dead->m_location, uf->m_opponent->m_id);
  }
}

bool scenario_arena::active() {
  return s_active;
}

uint32_t scenario_arena::get_score(uint32_t player_id) {
  return s_player_score[player_id];
}

void scenario_arena::start() {
  if (s_active) return;

  unit::sub_destroy(dead_unit);
  s_active = true;
}

void scenario_arena::process() {
  auto city_func = [](City& cityInstance, Player& player) {
    unit::create(UNIT_TYPE::PHALANX, cityInstance.m_location, player.m_id);
  };
  auto player_func = [city_func](Player& player) {
    player::for_each_player_city(player.m_id,
      [city_func, &player](City& c) { city_func(c, player); });
  };
  // Important: for_each_player is an ordered std::set
  // We must process players in order for consistent simulation results
  player::for_each_player(player_func);
}

