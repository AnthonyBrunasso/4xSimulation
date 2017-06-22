
#include <stdint.h>
#include <string>
#include <vector>

#include "enum_generated.h"

const uint32_t SCIENCE_EDGES_MAX = 4;

class ScienceNode {
public:
  explicit ScienceNode()
  : m_type(fbs::SCIENCE_TYPE::UNKNOWN)
  { }

  std::string Name();
  bool Researched(uint32_t player_id);
  
  ScienceNode* m_previous[SCIENCE_EDGES_MAX];
  ScienceNode* m_next[SCIENCE_EDGES_MAX];
  fbs::SCIENCE_TYPE m_type;
};

namespace science {
  void initialize();
  void reset();

  ScienceNode* Science(fbs::SCIENCE_TYPE);
  ScienceNode* Science(uint32_t);
  float research_cost(ScienceNode* sn);
  void debug_requirements(ScienceNode* sn);
  bool available(uint32_t player_id, ScienceNode* sn);
  void research_complete(uint32_t player_id, ScienceNode* sn);
}

