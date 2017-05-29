
#include <stdint.h>
#include <string>
#include <vector>

namespace fbs {
  enum class SCIENCE_TYPE : uint32_t;
}

class ScienceNode {
public:
  explicit ScienceNode(fbs::SCIENCE_TYPE type)
  : m_type(type)
  { }

  std::string Name();
  bool Researched(uint32_t player_id);
  
  std::vector<ScienceNode*> m_previous;
  std::vector<ScienceNode*> m_next;
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

