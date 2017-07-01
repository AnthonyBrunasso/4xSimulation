#include <iostream>
#include <string>

#include "random.h"
#include "step_generated.h"
#include "player.h"
#include "ai_neural_net.h"
#include "ai_unit_decisions.h"
#include "world_map.h"
#include "unit.h"

#include "simulation_interface.h"

// Get hex map and flatten it into an array containining enemy units and ally units.
std::vector<float> get_input() {
  std::vector<float> input(world_map::get_map_size());
  int i = 0;
  world_map::for_each_tile([&input, &i] (const sf::Vector3i&, const Tile& tile) {
    if (tile.m_unit_ids.empty()) {
      input[i] = 0.0f;
      return;
    }

    for (auto id : tile.m_unit_ids) {
      Unit* u = unit::get_unit(id);
      if (!u) continue;
      if (u->m_id == 0) {
        // Enemy
        input[i] = 1.0f;
      }
      else {
        // Ally
        input[i] = 0.5f;
      }
    }
    ++i;
  });

  return std::move(input);
}

int main(int, char*[]) {
  simulation_start();

  simulation_join_barbarian("barbarian");
  simulation_join_player("learner");

  simulation_barbarians_set_id(0);

  uint32_t nn_id = neural_net::create({ world_map::get_map_size(), 2 }, { &unit_decisions::get_wander(), &unit_decisions::get_fight() });
  neural_net::set_player_id(1, nn_id);

  simulation_start_faceoff();

  bool game_over = false;
  uint32_t loser = -1;
  while (!game_over) {
    simulation_barbarians_execute_turn(0);
    
    simulation_end_turn(0, 1);

    neural_net::execute(1, get_input());

    simulation_end_turn(1, 0);

    // Terminate when someone has no units.

    player::for_each_player([&game_over, &loser](Player& player) {
      if (player.m_units.empty()) {
        game_over = true;
        loser = player.m_id;
      }
    });
  }

  std::cout << "Graceful game end loser is: " << loser << std::endl;
  // Enter interactive mode
  std::string input;
  std::getline(std::cin, input);

  simulation_end();
  simulation_barbarians_reset();

  return 0;
}
