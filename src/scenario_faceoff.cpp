#include "scenario_faceoff.h"
#include "ai_shared.h"
#include "player.h"
#include "city.h"
#include "unit.h"
#include <unordered_map>
#include <list>

namespace scenario_faceoff
{
  typedef std::unordered_map<uint32_t, uint32_t> PlayerScore;
  PlayerScore s_player_score;
  bool s_active;

  uint32_t score(UNIT_TYPE ut);
  void dead_unit(UnitFatality* uf);
};

uint32_t scenario_faceoff::score(UNIT_TYPE ut)
{
  return 1;
}

void scenario_faceoff::dead_unit(UnitFatality* uf)
{
  // Don't score units that didn't have an opposing player
  if (!uf->m_opponent) return;

  s_player_score[uf->m_opponent->m_id] += score(uf->m_dead->m_type);
}

bool scenario_faceoff::active() {
  return s_active;
}

uint32_t scenario_faceoff::get_score(uint32_t player_id) {
  return s_player_score[player_id];
}

void scenario_faceoff::start() {
  if (s_active) return;

  if (player::get_count() != 2) return;

  std::vector<uint32_t> unit_types = { util::enum_to_uint(UNIT_TYPE::ARCHER),
    util::enum_to_uint(UNIT_TYPE::ARCHER),
    util::enum_to_uint(UNIT_TYPE::PHALANX),
    util::enum_to_uint(UNIT_TYPE::SCOUT),
    util::enum_to_uint(UNIT_TYPE::PHALANX)};
  std::vector<sf::Vector3i> p0_locations = { sf::Vector3i(5, -10, 5),
    sf::Vector3i(6, -10, 4),
    sf::Vector3i(4, -9, 5),
    sf::Vector3i(5, -9, 4),
    sf::Vector3i(6, -9, 3) };
  std::vector<sf::Vector3i> p1_locations = { sf::Vector3i(-5, 9, -4),
    sf::Vector3i(-4, 9, -5),
    sf::Vector3i(-5, 8, -3),
    sf::Vector3i(-4, 8, -4),
    sf::Vector3i(-3, 8, -5) };

  for (int i = 0; i < p0_locations.size(); ++i)
  {
    unit::create(util::uint_to_enum<UNIT_TYPE>(unit_types[i]), p0_locations[i], 0);
  }
  for (int i = 0; i < p1_locations.size(); ++i)
  {
    unit::create(util::uint_to_enum<UNIT_TYPE>(unit_types[i]), p1_locations[i], 1);
  }

  unit::sub_destroy(dead_unit);
  s_active = true;
}

void scenario_faceoff::process() {
  // No special rules 
}

void scenario_faceoff::reset() {
  s_player_score.clear();
  s_active = false;
}

