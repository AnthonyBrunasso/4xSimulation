#include "status_effect.h"

#include <iostream>
#include <map>
#include <utility>

#include "city.h"
#include "search.h"
#include "step_generated.h"
#include "tile.h"
#include "unit.h"
#include "world_map.h"

namespace {
  // Use this to trigger early status effect termination
  const uint32_t INVALID_TURNS = 0xffffffff;

  // Helper functions for status effect implementations.
  void spread_tile(Tile& tile, std::vector<sf::Vector3i>& tiles) {
    tiles.push_back(tile.m_location);
  }

  void spread_units(Tile& tile, std::vector<uint32_t>& units) {
    for (auto u : tile.m_unit_ids) {
      if (!unit::get_unit(u)) continue;
      units.push_back(u);
    }
  }

  void spread_city(Tile& tile, std::vector<uint32_t>& cities) {
    if (!city::get_city(tile.m_city_id)) return;
    cities.push_back(tile.m_city_id);
  }

  void spread_all(Tile& tile
      , std::vector<sf::Vector3i>& tiles
      , std::vector<uint32_t>& units
      , std::vector<uint32_t>& cities) {
    spread_tile(tile, tiles);
    spread_units(tile, units);
    spread_city(tile, cities);
  }

  class SummoningMonster : public StatusEffect {
  public:
    SummoningMonster(uint32_t id, fbs::STATUS_TYPE type, const sf::Vector3i& location) :
        StatusEffect(id, type, location) {
      m_turns = 20; 
      m_current_turn = 20;
    };

    void end_turn() override {
      std::cout << "MONSTER GRRRRRRRR" << std::endl;
    }

    void spread(Tile& /*tile*/) override {

    }
  };
  
  // Status effect implementations, maybe these should be moved to another file.
  class StasisEffect : public StatusEffect {
  public:
    StasisEffect(uint32_t id, fbs::STATUS_TYPE type, const sf::Vector3i& location) :
        StatusEffect(id, type, location) {
      m_range = 1;
      m_turns = 2;
      m_current_turn = 2;
    };

    void per_turn() override {
      for (auto id : m_units) {
        Unit* u = unit::get_unit(id);
        if (!u) continue;
        u->m_action_points = 0;
      }

      std::cout << "status: " << fbs::EnumNameSTATUS_TYPE(m_type) 
        << " id: " << m_id 
        << " evaluting per turn logic. " << std::endl;
    }

    void spread(Tile& tile) override {
      ::spread_all(tile, m_tiles, m_units, m_cities);
    }
  };

  class ConstructingEffect : public StatusEffect {
  public:
    ConstructingEffect(uint32_t id, fbs::STATUS_TYPE type, const sf::Vector3i& location) :
        StatusEffect(id, type, location) {
      // Only effects the tile it is on, therefore range 0.
      m_range = 0;
      // Takes two turns to construct.
      m_turns = 2;
      m_current_turn = 2;
    };

    void per_turn() override {
      for (auto u : m_units) {
        Unit* unit = unit::get_unit(u);
        if (!unit) continue;
        if (unit->m_type != fbs::UNIT_TYPE::WORKER) continue;
        // Workers get depleted action points per turn.
        unit->m_action_points = 0;
      }
    }

    void spread(Tile& tile) override {
      // TODO: Really this can only spread to a worker.
      ::spread_units(tile, m_units);
    }

    fbs::IMPROVEMENT_TYPE m_improvement_type;
  };


  typedef std::map<uint32_t, StatusEffect*> StatusMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;

  StatusMap s_status;
  SubMap s_create_subs;
  SubMap s_destroy_subs;

  std::function<void()> s_injected_begin = {};
  std::function<void()> s_injected_end = {};
  std::function<void()> s_injected_per = {};

  // Status effects don't use the global unique_id ids.
  uint32_t s_unique_ids = 1;
}

void status_effect::inject(std::function<void()> begin
    , std::function<void()> end
    , std::function<void()> per) {
  s_injected_begin = begin;
  s_injected_end = end;
  s_injected_per = per;
}

void status_effect::inject_begin(std::function<void()> begin) {
  s_injected_begin = begin;
}

void status_effect::inject_end(std::function<void()> end) {
  s_injected_end = end;
}

void status_effect::inject_per(std::function<void()> per) {
  s_injected_per = per;
}

uint32_t status_effect::create(fbs::STATUS_TYPE type, const sf::Vector3i& location) {
  uint32_t id = s_unique_ids++;
  StatusEffect* e = nullptr;
  // Best factory, ever. 
  switch (type) {
    case fbs::STATUS_TYPE::STASIS:
    e = new StasisEffect(id, type, location);
    break;
    case fbs::STATUS_TYPE::CONSTRUCTING_IMPROVEMENT:
    e = new ConstructingEffect(id, type, location);
    break;
    case fbs::STATUS_TYPE::SUMMONING_MONSTER:
    e = new SummoningMonster(id, type, location);
    break;
    case fbs::STATUS_TYPE::RESIST_MODIFIERS:
    case fbs::STATUS_TYPE::UNKNOWN:
    return 0;
  }

  if (!e) return 0;

  e->m_begin_turn_injection = s_injected_begin;
  e->m_end_turn_injection = s_injected_end;
  e->m_per_turn_injection = s_injected_per;

  s_status[id] = e;
  std::cout << "Created status effect id " << id << " type: " << fbs::EnumNameSTATUS_TYPE(type) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  // Gather all tiles in range of status effect.
  std::vector<sf::Vector3i> tiles;
  auto gather = [&tiles](const Tile& tile) -> bool { 
    tiles.push_back(tile.m_location);
    return false; 
  };
  search::bfs(location, e->m_range, world_map::get_map(), gather);

  // Spread the plague! Or, you know, whatever.
  for (auto loc : tiles) {
    Tile* t = world_map::get_tile(loc);
    if (!t) continue;
    e->spread(*t);
  }

  // Clear the injections.
  s_injected_begin = {};
  s_injected_end = {};
  s_injected_per = {};

  return id;
}

void status_effect::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void status_effect::destroy(uint32_t id) {
  StatusEffect* e = get_effect(id);
  if (!e) return;

  for (auto sub : s_destroy_subs) {
    sub(e->m_location, id);
  }

  s_status.erase(id);
  delete e;
}
void status_effect::sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_destroy_subs.push_back(sub);
}

StatusEffect* status_effect::get_effect(uint32_t id) {
  if (s_status.find(id) == s_status.end()) return nullptr;
  return s_status[id];
}

void status_effect::for_each_effect(std::function<void(const StatusEffect& effect)> operation) {
  for (auto status : s_status) {
    operation(*status.second);
  }
}

void status_effect::process() {
  std::vector<uint32_t> remove_effects;
  // Prune effects list of nullptrs and those flagged with INVALID_TURNS to be removed.
  for (auto effect : s_status) {
    StatusEffect* e = effect.second;
    if (!e || e->m_current_turn == INVALID_TURNS) {
      remove_effects.push_back(effect.first);
      delete e;
    }
  }

  for (auto remove_id : remove_effects) {
    s_status.erase(s_status.find(remove_id));
  }

  // Clear the list so it can be used for natural removal
  remove_effects.clear();

  // Guarantee that all begin turn effects happen before per turn effects.
  for (auto effect : s_status) {
    StatusEffect* e = effect.second;
    if (!e) continue;

    // Process begin turn logic. 
    if (e->m_turns == e->m_current_turn) {
      e->begin_turn();
      if (e->m_begin_turn_injection) e->m_begin_turn_injection();
    }
  }

  // Likewise, guarantee all per turn effects happen before end turn effects.
  for (auto effect : s_status) {
    StatusEffect* e = effect.second;
    if (!e) continue;
    e->per_turn();
    if (e->m_per_turn_injection) e->m_per_turn_injection();
  }

  for (auto effect : s_status) {
    StatusEffect* e = effect.second;
    if (!e) continue;
    if (e->m_current_turn == 0) {
      e->end_turn();
      remove_effects.push_back(effect.first);
      if (e->m_end_turn_injection) e->m_end_turn_injection();
      delete e;
    }
  }

  // Remove effects that have reached turn 0 and executed their end turn logic.
  for (auto remove_id : remove_effects) {
    s_status.erase(s_status.find(remove_id));
  }

  // Decrement the current turn for all status effects.
  for (auto& effect : s_status) {
    StatusEffect* e = effect.second;
    if (!e) continue;
    --e->m_current_turn;
  } 
}

void status_effect::reset() {
  for (auto& s : s_status) {
    delete s.second;  
  }
  s_status.clear();

  s_create_subs.clear();
  s_destroy_subs.clear();
}
