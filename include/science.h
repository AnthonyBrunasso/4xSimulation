
#include <vector>
#include <string>

enum class SCIENCE_TYPE;

class ScienceNode {
public:
  explicit ScienceNode(std::string&& name)
  : m_name(name)
  { }

  std::vector<ScienceNode*> m_previous;
  std::vector<ScienceNode*> m_next;
  bool m_researched;
  std::string m_name;
};

namespace science {
  void initialize();
  void shutdown();

  ScienceNode* Science(SCIENCE_TYPE);
  void debug_requirements(ScienceNode* sn);
  bool available(ScienceNode* sn);
}

