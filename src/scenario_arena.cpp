#include "scenario_arena.h"

#include <cstdint>
#include <list>
#include <memory>
#include <unordered_map>
#include <utility>

#include "city.h"

#include "player.h"
#include "step_generated.h"
#include "unit.h"
#include "util.h"

namespace scenario_arena
{
  typedef std::unordered_map<uint32_t, uint32_t> PlayerScore;
  PlayerScore s_player_score;
  typedef std::list<std::pair<uint32_t, uint32_t> > SpawnQueue;
  SpawnQueue s_spawn_list;
  bool s_active;

  uint32_t score(fbs::UNIT_TYPE ut);
  void dead_unit(UnitFatality* uf);
};

uint32_t scenario_arena::score(fbs::UNIT_TYPE ut)
{
  switch(ut)
  {
  case fbs::UNIT_TYPE::MONSTER:
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

  if ((s_player_score[uf->m_opponent->m_id] & (16 - 1)) == 0) {
    s_spawn_list.push_back(std::pair<uint32_t, uint32_t>(uf->m_opponent->m_id, any_enum(fbs::UNIT_TYPE::WIZARD)));
  }
  else if ((s_player_score[uf->m_opponent->m_id] & (8 - 1)) == 0) {
    s_spawn_list.push_back(std::pair<uint32_t, uint32_t>(uf->m_opponent->m_id, any_enum(fbs::UNIT_TYPE::ARCHER)));
  }
  else if ((s_player_score[uf->m_opponent->m_id] & (4-1)) == 0) {
    const auto& spawn_fn = [uf](const City& c) {
      unit::create(fbs::UNIT_TYPE::SCOUT, c.m_location, uf->m_opponent->m_id);
    };
    player::for_each_player_city(uf->m_opponent->m_id, spawn_fn);
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
    for (SpawnQueue::iterator it = s_spawn_list.begin(); it != s_spawn_list.end(); ++it) {
      if (it->first != player.m_id) continue;
      
      unit::create(any_enum(it->second), cityInstance.m_location, player.m_id);
      s_spawn_list.erase(it);
      return;
    }

    unit::create(fbs::UNIT_TYPE::PHALANX, cityInstance.m_location, player.m_id);
  };
  auto player_func = [city_func](Player& player) {
    player::for_each_player_city(player.m_id,
      [city_func, &player](City& c) { city_func(c, player); });
  };

  // Important: for_each_player is an ordered std::set
  // We must process players in order for consistent simulation results
  player::for_each_player(player_func);
}

void scenario_arena::reset() {
  s_player_score.clear();
  s_spawn_list.clear();
  s_active = false;
}

