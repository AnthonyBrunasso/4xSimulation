
#include "science.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>


#include "player.h"
#include "step_generated.h"

namespace science {
  class ScienceEdge;

  typedef std::unordered_map<uint32_t, ScienceNode*> ScienceNodeMap;

  ScienceNodeMap s_tower_of_babylon;
  std::vector<ScienceEdge> s_edges;

  ScienceNode* Science(fbs::SCIENCE_TYPE st)  {
    return Science(static_cast<uint32_t>(st));
  }

  ScienceNode* Science(uint32_t st_i) {
    ScienceNodeMap::const_iterator itFind = s_tower_of_babylon.find(st_i);
    if (itFind == s_tower_of_babylon.end()) return nullptr;
    return itFind->second;
  }
 
  class ScienceEdge {
  public:
    explicit ScienceEdge(fbs::SCIENCE_TYPE prev, fbs::SCIENCE_TYPE next) 
    : m_previous(Science(prev))
    , m_next(Science(next))
    {}

    ScienceNode* m_previous;
    ScienceNode* m_next;
  };

  void initialize() {
    auto science_init = [](fbs::SCIENCE_TYPE st) {
      if (st == fbs::SCIENCE_TYPE::UNKNOWN) return;
      uint32_t key = static_cast<uint32_t>(st);
      s_tower_of_babylon[key] = new ScienceNode(st);
    };
    for (const auto& science : fbs::EnumValuesSCIENCE_TYPE()) {
      science_init(science);
    }
    std::vector<ScienceEdge> edges { 
      ScienceEdge(fbs::SCIENCE_TYPE::AGRICULTURE,fbs::SCIENCE_TYPE::POTTERY),
      ScienceEdge(fbs::SCIENCE_TYPE::AGRICULTURE,fbs::SCIENCE_TYPE::ANIMAL_HUSBANDRY),
      ScienceEdge(fbs::SCIENCE_TYPE::AGRICULTURE,fbs::SCIENCE_TYPE::ARCHERY),
      ScienceEdge(fbs::SCIENCE_TYPE::AGRICULTURE,fbs::SCIENCE_TYPE::MINING),
      ScienceEdge(fbs::SCIENCE_TYPE::POTTERY,fbs::SCIENCE_TYPE::SAILING),
      ScienceEdge(fbs::SCIENCE_TYPE::POTTERY,fbs::SCIENCE_TYPE::CALENDAR),
      ScienceEdge(fbs::SCIENCE_TYPE::POTTERY,fbs::SCIENCE_TYPE::WRITING),
      ScienceEdge(fbs::SCIENCE_TYPE::ANIMAL_HUSBANDRY,fbs::SCIENCE_TYPE::TRAPPING),
      ScienceEdge(fbs::SCIENCE_TYPE::ANIMAL_HUSBANDRY,fbs::SCIENCE_TYPE::WHEEL),
      ScienceEdge(fbs::SCIENCE_TYPE::ARCHERY,fbs::SCIENCE_TYPE::WHEEL),
      ScienceEdge(fbs::SCIENCE_TYPE::MINING,fbs::SCIENCE_TYPE::MASONRY),
      ScienceEdge(fbs::SCIENCE_TYPE::MINING,fbs::SCIENCE_TYPE::BRONZE_WORKING)
    };
    for (auto e : edges) {
      e.m_previous->m_next.push_back(e.m_next);
      e.m_next->m_previous.push_back(e.m_previous);
    }
    s_edges.swap(edges);
  }

  void debug_requirements(ScienceNode* sn) {
    if (!sn) return;
    std::cout << "Science Required: " << sn->Name() << std::endl;
    for (auto node : sn->m_previous) {
      debug_requirements(node);
    }
  }

  bool available(uint32_t player_id, ScienceNode* sn) {
    if(!sn) return false;
    bool can_research = true;
    for (auto node : sn->m_previous) {
      can_research = can_research && node->Researched(player_id);
    }
    return can_research;
  }

  void research_complete(uint32_t player_id, ScienceNode* sn) {
    if (!sn) return;
    Player* player = player::get_player(player_id);
    if(!player) return;
    uint32_t st_i = static_cast<uint32_t>(sn->m_type);
    std::vector<uint32_t>::iterator findIt = std::find(player->m_available_research.begin(), player->m_available_research.end(), st_i);
    if (findIt == player->m_available_research.end()) return;
    player->m_available_research.erase(findIt);
    player->m_discovered_science.insert(st_i);
    for (auto node : sn->m_next) {
      if (available(player_id, node)) {
        player->m_available_research.push_back(static_cast<uint32_t>(node->m_type));
      }
    }
  }
  
  uint32_t node_depth(ScienceNode* sn, uint32_t depth=1) {
    if (sn->m_previous.empty()) return depth;
    uint32_t min_depth = 0xffffffff;
    for (auto node : sn->m_previous) {
      min_depth = std::min(min_depth, node_depth(node, depth+1));
    }
    return min_depth;
  }
  
  float ancient_exponent(uint32_t x) {
    return 43.f - 18.f*x + 14.f * std::pow(x,2.0f);
  }

  float normal_exponent(uint32_t x) {
    return 270.f - 20.f*x+72.f*std::pow(x, 2.0f);
  }
  
  float research_cost(ScienceNode* sn) {
    uint32_t depth = node_depth(sn) - 1;
    if (depth == 0) {
      return 23.f;
    }
    else if (depth <= 4) {
      return ancient_exponent(depth);
    }

    return normal_exponent(depth-4);
  }

  void reset() {
    for (auto& s : s_tower_of_babylon) {
      delete s.second;
    }

    s_tower_of_babylon.clear();
    s_edges.clear();
  }
};

std::string ScienceNode::Name() {
  return fbs::EnumNameSCIENCE_TYPE(m_type);
}

bool ScienceNode::Researched(uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) return false;
  return player->DiscoveredScience(m_type);
}
