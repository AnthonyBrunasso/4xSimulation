#include "ai_decisions.h"

#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include "step.h"
#include "tile.h"
#include "player.h"
#include "city.h"
#include "simulation.h"
#include "Vector3.hpp"
#include "format.h"
#include "game_types.h"
#include "util.h"
#include "random.h"
#include "world_map.h"
#include "city.h"
#include "production.h"

void Settle::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Unable to find player to settle." << std::endl;
  }

  std::unordered_map<sf::Vector3i, bool> invalid_homes;   
  player::for_each_player([&invalid_homes](Player& player) {
    for (auto id : player.m_cities) {
      City* city = city::get_city(id);
      if (!city) continue;
      invalid_homes[city->m_location] = true;
    }
  });

  int start = 7;
  // Find a home.
  sf::Vector3i new_home = game_random::cube_coord(start);
  Tile* tile = world_map::get_tile(new_home);
  while (invalid_homes.find(new_home) != invalid_homes.end() || !tile) {
    new_home = game_random::cube_coord(start);
    tile = world_map::get_tile(new_home); 
  }

  std::cout << current->m_name << " found a home at " << format::vector3(new_home) << std::endl;
  // Create a worker, for now, on that tile then a city.
  SpawnStep* spawn_step = new SpawnStep(COMMAND::SPAWN);
  spawn_step->m_unit_type = util::enum_to_uint(UNIT_TYPE::WORKER);
  spawn_step->m_location = new_home;
  spawn_step->m_player = player_id;
  simulation::process_step_from_ai(spawn_step);

  ColonizeStep* colonize_step = new ColonizeStep(COMMAND::COLONIZE);
  colonize_step->m_location = new_home;
  colonize_step->m_player = player_id;
  simulation::process_step_from_ai(colonize_step);
}

void Construct::operator()(uint32_t player_id) {
  Player* current = player::get_player(player_id);
  if (!current) {
    std::cout << "Unable to find player to construct." << std::endl;
  }

  for (auto id : current->m_cities) {
    City* city = city::get_city(id);
    if (!city) continue;
    // Begin construction of the production type in all available cities.
    if (!city->IsConstructing()) {
      std::cout << current->m_name << " beginning construction of: " << get_construction_name(m_production_type) <<
        " in city: " << city->m_id << std::endl;
      city->GetConstruction()->Add(m_production_type);
    }
  }
}

void Explore::operator()(uint32_t player_id) {
  std::cout << "Explore: " << player_id << std::endl;
}

