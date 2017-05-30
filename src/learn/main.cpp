#include <iostream>
#include <string>

#include "random.h"
#include "step_generated.h"
#include "player.h"

#include "simulation_interface.h"

int main(int, char*[]) {
  simulation_start();

  simulation_join_barbarian("barbarian");
  simulation_join_player("learner");

  simulation_barbarians_set_id(0);

  simulation_interface::start_faceoff();

  bool game_over = false;
  uint32_t loser = -1;
  while (!game_over) {
    simulation_barbarians_execute_turn(0);
    
    simulation_end_turn(0, 1);

    // Do learning stuff.

    simulation_end_turn(1, 0);

    // Terminate when someone has no units.

    player::for_each_player([&game_over, &loser](Player& player) {
      if (player.m_units.empty()) {
        game_over = true;
        loser = player.m_id;
      }
    });
  }

  std::cout << "Graceful game end." << std::endl;
  // Enter interactive mode
  std::string input;
  std::getline(std::cin, input);

  simulation_end();
  simulation_barbarians_reset();

  return 0;
}
