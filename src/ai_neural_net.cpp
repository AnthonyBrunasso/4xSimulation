#include "ai_neural_net.h"

#include "neural_net.h"
#include "unique_id.h"
#include "dtree.h"

#include <cassert>
#include <unordered_map>

namespace neural_net {

typedef std::unordered_map<uint32_t, NeuralNet<float>* > NNMap;
typedef std::unordered_map<uint32_t, std::vector<Decision*> > NNOutputs;
typedef std::unordered_map<uint32_t, uint32_t> NNPlayers;

NNMap s_nets;
NNOutputs s_outputs;
NNPlayers s_players;

void initialize() {
}

void reset() {
}

uint32_t create(const std::vector<size_t>& structure, const std::vector<Decision*>& decisions) {
  // This must be true or you're going to have a bad time.
  assert(structure.back() == decisions.size());
  NeuralNet<float>* net = new NeuralNet<float>(structure);
  uint32_t id = unique_id::generate();
  s_nets[id] = net;
  s_outputs[id] = decisions;
  return id;
}

void set_player_id(uint32_t player_id, uint32_t neural_net_id) {
  s_players[player_id] = neural_net_id;
}

void execute(uint32_t player_id, const std::vector<float>& input) {
  uint32_t net_id = s_players[player_id];
  NeuralNet<float>* nn = s_nets[net_id];
  std::vector<float> output = nn->predict(input);
  for (auto& o : output) {
    std::cout << o << std::endl;
  }
}

}
