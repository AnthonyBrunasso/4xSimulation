#pragma once

class Decision;

#include <cstddef>
#include <stdint.h>
#include <vector>

namespace neural_net {

void initialize();
void reset();

// The structures first entry must correspond with the size of the input the neural network will get.
// The final entry must correspond with how many different decisions the neural network can make.
uint32_t create(const std::vector<size_t>& structure, const std::vector<Decision*>& decisions);
void set_player_id(uint32_t player_id, uint32_t neural_net_id);
// Execute the neural network associated with player_id using some input
void execute(uint32_t player_id, const std::vector<float>& input);

}
