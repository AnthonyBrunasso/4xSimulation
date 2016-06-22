
#include <vector>
#include <string>

enum class SCIENCE_TYPE;

class ScienceNode {
public:
  explicit ScienceNode(SCIENCE_TYPE type)
  : m_type(type)
  { }

  std::string Name();
  bool Researched(uint32_t player_id);
  
  std::vector<ScienceNode*> m_previous;
  std::vector<ScienceNode*> m_next;
  SCIENCE_TYPE m_type;
};

namespace science {
  void initialize();
  void shutdown();

  ScienceNode* Science(SCIENCE_TYPE);
  void debug_requirements(ScienceNode* sn);
  bool available(uint32_t player_id, ScienceNode* sn);
}

