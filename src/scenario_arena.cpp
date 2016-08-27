#include "scenario_arena.h"
#include "player.h"
#include "city.h"
#include "unit.h"

void scenario_arena::start() {
}

void scenario_arena::process() {
  auto city_func = [](City& cityInstance, Player& player) {
    unit::create(UNIT_TYPE::ARCHER, cityInstance.m_location, player.m_id);
  };
  auto player_func = [city_func](Player& player) {
    player::for_each_player_city(player.m_id,
      [city_func, &player](City& c) { city_func(c, player); });
  };
  // Important: for_each_player is an ordered std::set
  // We must process players in order for consistent simulation results
  player::for_each_player(player_func);
}

