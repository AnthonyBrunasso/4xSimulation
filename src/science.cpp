
#include "science.h"
#include "game_types.h"
#include "player.h"
#include <algorithm>
#include <unordered_map>
#include <iostream>

namespace science {
  class ScienceEdge;
  typedef std::unordered_map<uint32_t, ScienceNode*> ScienceNodeMap;

  ScienceNodeMap s_tower_of_babylon;
  std::vector<ScienceEdge> s_edges;

  ScienceNode* Science(SCIENCE_TYPE st)  {
    return Science(static_cast<uint32_t>(st));
  }

  ScienceNode* Science(uint32_t st_i) {
    ScienceNodeMap::const_iterator itFind = s_tower_of_babylon.find(st_i);
    if (itFind == s_tower_of_babylon.end()) return nullptr;
    return itFind->second;
  }
 
  class ScienceEdge {
  public:
    explicit ScienceEdge(SCIENCE_TYPE prev, SCIENCE_TYPE next) 
    : m_previous(Science(prev))
    , m_next(Science(next))
    {}

    ScienceNode* m_previous;
    ScienceNode* m_next;
  };

  void initialize() {
    auto science_init = [](SCIENCE_TYPE st) {
      uint32_t key = static_cast<uint32_t>(st);
      s_tower_of_babylon[key] = new ScienceNode(st);
    };
    for_each_science_type(science_init);
    std::vector<ScienceEdge> edges { 
      ScienceEdge(SCIENCE_TYPE::AGRICULTURE,SCIENCE_TYPE::POTTERY),
      ScienceEdge(SCIENCE_TYPE::AGRICULTURE,SCIENCE_TYPE::ANIMAL_HUSBANDRY),
      ScienceEdge(SCIENCE_TYPE::AGRICULTURE,SCIENCE_TYPE::ARCHERY),
      ScienceEdge(SCIENCE_TYPE::AGRICULTURE,SCIENCE_TYPE::MINING),
      ScienceEdge(SCIENCE_TYPE::POTTERY,SCIENCE_TYPE::SAILING),
      ScienceEdge(SCIENCE_TYPE::POTTERY,SCIENCE_TYPE::CALENDAR),
      ScienceEdge(SCIENCE_TYPE::POTTERY,SCIENCE_TYPE::WRITING),
      ScienceEdge(SCIENCE_TYPE::ANIMAL_HUSBANDRY,SCIENCE_TYPE::TRAPPING),
      ScienceEdge(SCIENCE_TYPE::ANIMAL_HUSBANDRY,SCIENCE_TYPE::WHEEL),
      ScienceEdge(SCIENCE_TYPE::ARCHERY,SCIENCE_TYPE::WHEEL),
      ScienceEdge(SCIENCE_TYPE::MINING,SCIENCE_TYPE::MASONRY),
      ScienceEdge(SCIENCE_TYPE::MINING,SCIENCE_TYPE::BRONZE_WORKING)
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
    std::vector<uint32_t>::iterator findIt = std::find(player->m_available_science.begin(), player->m_available_science.end(), st_i);
    if (findIt == player->m_available_science.end()) return;
    player->m_available_science.erase(findIt);
    player->m_discovered_science.insert(st_i);
    for (auto node : sn->m_next) {
      if (available(player_id, node)) {
        player->m_available_science.push_back(static_cast<uint32_t>(node->m_type));
      }
    }
  }
  
  void shutdown() {
    ScienceNodeMap::iterator it = s_tower_of_babylon.begin();
    for(; it != s_tower_of_babylon.end(); ++it) {
      delete it->second;
    }
    // Burn baby burn
    s_tower_of_babylon.clear();
  }
};

std::string ScienceNode::Name() {
  return get_science_name(m_type);
}

bool ScienceNode::Researched(uint32_t player_id) {
  Player* player = player::get_player(player_id);
  if (!player) return false;
  return player->DiscoveredScience(m_type);
}

