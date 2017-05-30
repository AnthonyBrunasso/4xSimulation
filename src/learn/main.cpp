#include <iostream>
#include <string>

#include "simulation.h"
#include "ai_barbarians.h"
#include "random.h"
#include "step_generated.h"
#include "player.h"

#include "simulation_interface.h"

int main(int, char*[]) {
  simulation_interface::start();

  simulation_interface::join_player(fbs::AI_TYPE::BARBARIAN, "barbarian");
  simulation_interface::join_player(fbs::AI_TYPE::HUMAN, "learner");

  barbarians::set_player_id(0);

  simulation_interface::start_faceoff();

  bool game_over = false;
  uint32_t loser = -1;
  while (!game_over) {
    barbarians::pillage_and_plunder(0);
    
    simulation_interface::end_turn(0, 1);

    // Do learning stuff.

    simulation_interface::end_turn(1, 0);

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

  simulation::kill();
  barbarians::reset();

  return 0;
}
