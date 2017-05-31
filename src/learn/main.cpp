#include <iostream>
#include <string>

#include "random.h"
#include "step_generated.h"
#include "player.h"
#include "ai_neural_net.h"
#include "ai_unit_decisions.h"

#include "simulation_interface.h"

int main(int, char*[]) {
  simulation_start();

  simulation_join_barbarian("barbarian");
  simulation_join_player("learner");

  simulation_barbarians_set_id(0);

  uint32_t nn_id = neural_net::create({ 2, 1 }, { &unit_decisions::get_wander() });
  neural_net::set_player_id(1, nn_id);

  simulation_start_faceoff();

  bool game_over = false;
  uint32_t loser = -1;
  while (!game_over) {
    simulation_barbarians_execute_turn(0);
    
    simulation_end_turn(0, 1);

    // Do learning stuff.
    neural_net::execute(1, { 1.0f, 2.0f });

    simulation_end_turn(1, 0);

    // Terminate when someone has no units.

    player::for_each_player([&game_over, &loser](Player& player) {
      if (player.m_units.empty()) {
        game_over = true;
        loser = player.m_id;
      }
    });

    std::string input;
    std::getline(std::cin, input);
  }

  std::cout << "Graceful game end." << std::endl;
  // Enter interactive mode
  std::string input;
  std::getline(std::cin, input);

  simulation_end();
  simulation_barbarians_reset();

  return 0;
}
