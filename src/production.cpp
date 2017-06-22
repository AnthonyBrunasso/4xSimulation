
#include "production.h"

#include <string.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>

#include "Vector3.hpp"
#include "city.h"
#include "entity.h"
#include "player.h"
#include "production_detail.h"
#include "terrain_yield.h"
#include "unique_id.h"
#include "unit.h"
#include "util.h"

static constexpr size_t ORDER_LIMIT = (size_t)fbs::CONSTRUCTION_TYPE::MAX+1;
class ConstructionState
{
public:
  ConstructionState();
  ~ConstructionState();

  ConstructionState(ConstructionState&&) = default;
  ConstructionState(const ConstructionState&) = delete;
  ConstructionState& operator=(const ConstructionState&) = delete;

  uint32_t GetConstruction(fbs::CONSTRUCTION_TYPE type_id, uint32_t city_id);
  bool EraseConstruction(fbs::CONSTRUCTION_TYPE type_id);
  bool IsConstructed(fbs::CONSTRUCTION_TYPE type_id) const;

private:
  friend std::ostream& operator<<(std::ostream&, const ConstructionState&);

  uint32_t m_constructions[ORDER_LIMIT];
};
std::ostream& operator<<(std::ostream&, const ConstructionState&);

ECS_COMPONENT(ConstructionOrder, 256);
ECS_COMPONENT(ConstructionQueueFIFO, 128);
ECS_COMPONENT(ConstructionState, 128);

namespace production {
  bool production_cleanup(const sf::Vector3i&, uint32_t city_id) {
    City* city = city::get_city(city_id);
    if (!city) return false;

    uint32_t c = delete_c(city_id, s_ConstructionQueueFIFO());
    if (c == INVALID_COMPONENT) return false;
    
    c = delete_c(city_id, s_ConstructionState());
    if (c == INVALID_COMPONENT) return false;

    return true;
  }
  
  uint32_t create(uint32_t city_id) {
    // Binding production to the same entity as city
    uint32_t c = create(city_id, s_ConstructionQueueFIFO());
    ConstructionQueueFIFO* cq = c_ConstructionQueueFIFO(c);
    cq->m_city_id = city_id;

    // Binding construction state to the same entity as city
    c = create(city_id, s_ConstructionState());
    cq->m_state = c_ConstructionState(c);
    city::sub_raze_complete(production_cleanup);
    return city_id; // todo: not needed, now that id is shared
  }

  ConstructionQueueFIFO* get_production(uint32_t production_id) {
    uint32_t c = get(production_id, s_ConstructionQueueFIFO());
    return c_ConstructionQueueFIFO(c);
  }

  fbs::CONSTRUCTION_TYPE id(uint32_t type_id) {
    return any_enum(type_id);
  }

  const char*name(ConstructionOrder* co) {
    return fbs::EnumNameCONSTRUCTION_TYPE(co->m_type);
  }

  float current(ConstructionOrder* co) {
    return co->m_production;
  }

  float remains(ConstructionOrder* co) {
    return required(co->m_type) - co->m_production;
  }

  float required(ConstructionOrder* co) {
    return required(co->m_type);
  }

  float turns(ConstructionOrder* co) {
    City* c = city::get_city(co->m_city_id);
    if (!c) return 0.0;
    TerrainYield t = c->DumpYields();

    return std::ceil(production::remains(co) / t.m_production);
  }

  float apply(ConstructionOrder* co, float amount_available) {
    float required = production::required(co);
    float current = co->m_production;

    float usedProduction = std::min(amount_available, required - current);
    //std::cout << "used production: " << usedProduction << std::endl;

    co->m_production += usedProduction;
    //std::cout << "current production " << current << " / " << required << std::endl;
    return amount_available - usedProduction;
  }

  bool completed(ConstructionOrder* co) {
    return co->m_production >= required(co->m_type);
  }

  float required(fbs::CONSTRUCTION_TYPE type) {
    switch (type) {
    case fbs::CONSTRUCTION_TYPE::GRANARY:
      return 40.f;
    case fbs::CONSTRUCTION_TYPE::RANGE :
      return 30.f;
    case fbs::CONSTRUCTION_TYPE::FORGE :
      return 12.f;
    case fbs::CONSTRUCTION_TYPE::MELEE :
      return 30.f;
    case fbs::CONSTRUCTION_TYPE::FACTORY :
      return 60.f;
    case fbs::CONSTRUCTION_TYPE::SCOUT :
      return 15.f;
    case fbs::CONSTRUCTION_TYPE::WORKER :
      return 20.f;
    case fbs::CONSTRUCTION_TYPE::CASTER:
      return 40;
    case fbs::CONSTRUCTION_TYPE::UNKNOWN :
    default:
      return 1000.f;
    }
  }

  float required_to_purchase(fbs::CONSTRUCTION_TYPE type) {
    return required(type) * 3.f;
  }
  
  float yield_from_sale(fbs::CONSTRUCTION_TYPE type) {
    return required_to_purchase(type) * .25f;
  }

  bool construction_is_unique(fbs::CONSTRUCTION_TYPE type_id) {
    uint32_t type = any_enum(type_id);
    return (type & 1) != 0;
  }

  void spawn_unit(fbs::CONSTRUCTION_TYPE type_id, uint32_t city_id) {
    City* city = city::get_city(city_id);
    if (!city) return;

    uint32_t unit_id;
    switch (type_id) {
    case fbs::CONSTRUCTION_TYPE::SCOUT:
      unit_id = unit::create(fbs::UNIT_TYPE::SCOUT, city->m_location, city->m_owner_id);
      break;
    case fbs::CONSTRUCTION_TYPE::RANGE:
      unit_id = unit::create(fbs::UNIT_TYPE::ARCHER, city->m_location, city->m_owner_id);
      break;
    case fbs::CONSTRUCTION_TYPE::MELEE:
      unit_id = unit::create(fbs::UNIT_TYPE::PHALANX, city->m_location, city->m_owner_id);
      break;
    case fbs::CONSTRUCTION_TYPE::WORKER:
      unit_id = unit::create(fbs::UNIT_TYPE::WORKER, city->m_location, city->m_owner_id);
      break;
    case fbs::CONSTRUCTION_TYPE::CASTER:
      unit_id = unit::create(fbs::UNIT_TYPE::WIZARD, city->m_location, city->m_owner_id);
      break;
    default:
      return;
    }
    player::add_unit(city->m_owner_id, unit_id);
  }

  void reset() {
    for (auto qm : mapping_ConstructionQueueFIFO) {
      if (qm.entity == INVALID_ENTITY) continue;
      delete_c(qm.component, s_ConstructionQueueFIFO());
    }
    for (auto cm : mapping_ConstructionState) {
      if (cm.entity == INVALID_ENTITY) continue;
      delete_c(cm.component, s_ConstructionState());
    }
  }
}

namespace production_queue {
  std::vector<uint32_t> complete(const ConstructionQueueFIFO* cq) {
    std::vector<uint32_t> constructed;
    auto check = [cq, &constructed](fbs::CONSTRUCTION_TYPE t) {
      if (!cq->m_state->IsConstructed(t)) return;
      constructed.push_back(any_enum(t));
    };
    for (auto ct : fbs::EnumValuesCONSTRUCTION_TYPE()) {
      check(ct);
    }
    return (constructed);
  }

  std::vector<uint32_t> incomplete(const ConstructionQueueFIFO* cq) {
    std::vector<uint32_t> incomplete;
    auto check = ([cq, &incomplete](fbs::CONSTRUCTION_TYPE t) {
      if (cq->m_state->IsConstructed(t)) return;
      incomplete.push_back(any_enum(t));
    });
    for (auto ct : fbs::EnumValuesCONSTRUCTION_TYPE()) {
      check(ct);
    }
    return (incomplete);
  }
  
  std::vector<uint32_t> available(const ConstructionQueueFIFO* cq) {
    std::vector<uint32_t> incomplete;
    std::vector<uint32_t> queued = queue(cq);
    auto check = ([cq, &queued, &incomplete](fbs::CONSTRUCTION_TYPE t) {
      if (production::construction_is_unique(t)) {
        if (cq->m_state->IsConstructed(t)) return;
        uint32_t production_id = any_enum(t);
        auto itFind = std::find(queued.begin(), queued.end(), production_id);
        if (itFind != queued.end()) return;
      }
      
      incomplete.push_back(any_enum(t));
    });
    for (auto ct : fbs::EnumValuesCONSTRUCTION_TYPE()) {
      check(ct);
    }
    return (incomplete);
  }
  
  std::vector<uint32_t> queue(const ConstructionQueueFIFO* cq) {
    std::vector<uint32_t> queue;
    for (auto& q : cq->m_queue) {
      queue.push_back(any_enum(q->m_type));
    }

    return (queue);
  }

  fbs::CONSTRUCTION_TYPE front(const ConstructionQueueFIFO* cq) {
    if (cq->m_queue.empty()) return fbs::CONSTRUCTION_TYPE::UNKNOWN;
    return cq->m_queue.front()->m_type;
  }

  bool built(const ConstructionQueueFIFO* cq, fbs::CONSTRUCTION_TYPE type_id) {
    return cq->m_state->IsConstructed(type_id);
  }

  TerrainYield yield(const ConstructionQueueFIFO* cq) {
    TerrainYield ty;
    ty.m_production = 1.f;

    if (built(cq, fbs::CONSTRUCTION_TYPE::FORGE)) {
      ty.m_production += 1.5f;
    }
    if (built(cq, fbs::CONSTRUCTION_TYPE::FACTORY)) {
      ty.m_production += 10.0f;
    }
    if (built(cq, fbs::CONSTRUCTION_TYPE::GRANARY)) {
      ty.m_food += 2.0f;
    }

    return ty;
  }

  void add(ConstructionQueueFIFO* cq, fbs::CONSTRUCTION_TYPE type_id) {
    if (type_id == fbs::CONSTRUCTION_TYPE::UNKNOWN) {
      std::cout << "Construction Add on unknown type" << std::endl;
      return;
    }

    uint32_t order = cq->m_state->GetConstruction(type_id, cq->m_city_id);
    uint32_t c = get(order, s_ConstructionOrder());
    cq->m_queue.push_back(c_ConstructionOrder(c));
  }

  void move(ConstructionQueueFIFO* cq, size_t src, size_t dest) {
    if (src >= cq->m_queue.size()) {
      std::cout << "Invalid source index, list swap aborted" << std::endl;
      return;
    }

    if (dest >= cq->m_queue.size()) {
      std::cout << "Invalid destination index, list swap aborted" << std::endl;
      return;
    }

    std::list<ConstructionOrder*>::iterator itFrom = cq->m_queue.begin();
    std::advance(itFrom, src);

    std::list<ConstructionOrder*>::iterator itTo = cq->m_queue.begin();
    std::advance(itTo, dest);

    std::swap(*itFrom, *itTo);

    std::cout << "Swap completed " << std::endl;
    std::cout << *cq << std::endl;
  }

  void remove(ConstructionQueueFIFO* cq, size_t offset) {
    if (offset >= cq->m_queue.size()) return;
    std::list<ConstructionOrder*>::iterator itr = cq->m_queue.begin();
    std::advance(itr, offset);
    cq->m_queue.erase(itr);
  }

  void purchase(ConstructionQueueFIFO* cq, fbs::CONSTRUCTION_TYPE type_id) {
    if (type_id == fbs::CONSTRUCTION_TYPE::UNKNOWN) {
      std::cout << "Construction purchase on unknown type" << std::endl;
      return;
    }
    
    if (!production::construction_is_unique(type_id)) {
      production::spawn_unit(type_id, cq->m_city_id);
      return;
    }

    uint32_t order = cq->m_state->GetConstruction(type_id, cq->m_city_id);
    uint32_t c = get(order, s_ConstructionOrder());
    production::apply(c_ConstructionOrder(c), 9999.f);
  }

  void sell(ConstructionQueueFIFO* cq, fbs::CONSTRUCTION_TYPE type_id) {
    if (type_id == fbs::CONSTRUCTION_TYPE::UNKNOWN) {
      std::cout << "Construction sale on unknown type" << std::endl;
      return;
    }

    if (!production::construction_is_unique(type_id)) {
      return;
    }

    cq->m_state->EraseConstruction(type_id);
  }

  void simulate(ConstructionQueueFIFO* cq, TerrainYield& t) {
    cq->m_stockpile += t.m_production;
    while (cq->m_queue.size() > 0) {
      ConstructionOrder* order = cq->m_queue.front();
      cq->m_stockpile = production::apply(order, cq->m_stockpile);
      if (production::completed(order)) {
        ConstructionOrder* completed = cq->m_queue.front();
        std::cout << "Construction completed: " << fbs::EnumNameCONSTRUCTION_TYPE(completed->m_type) << std::endl;
        cq->m_queue.pop_front();
        if (!production::construction_is_unique(order->m_type)) {
          // Unit spawn
          production::spawn_unit(completed->m_type, cq->m_city_id);
          delete completed;
        }
      }
      if (cq->m_stockpile < 1.f) {
        break;
      }
    }

    // Limit the city's reserve when no construction orders are given
    //  The yield may have increased due to completed construction
    cq->m_stockpile = std::min(cq->m_stockpile, t.m_production);
  }
}

ConstructionState::ConstructionState() {
  memset(m_constructions, 0, sizeof(m_constructions));
}

ConstructionState::~ConstructionState() {
  for (auto construction : m_constructions) {
    delete_c(construction, s_ConstructionOrder());
  }
  memset(m_constructions, 0, sizeof(m_constructions));
}

uint32_t ConstructionState::GetConstruction(fbs::CONSTRUCTION_TYPE type_id, uint32_t city_id) {
  if (!production::construction_is_unique(type_id)) {
    uint32_t id = unique_id::generate();
    uint32_t c = create(id, s_ConstructionOrder());
    ConstructionOrder* co = c_ConstructionOrder(c);
    co->m_type = type_id;
    co->m_city_id = city_id;
    return id;
  }
  
  uint32_t type = any_enum(type_id);
  if (m_constructions[type]) {
    std::cout << "Resuming unique construction: " << fbs::EnumNameCONSTRUCTION_TYPE(type_id) << std::endl;
    return m_constructions[type];
  }

  std::cout << "New unique construction: " << fbs::EnumNameCONSTRUCTION_TYPE(type_id) << std::endl; 
  uint32_t id = unique_id::generate();
  uint32_t c = create(id, s_ConstructionOrder());
  ConstructionOrder* newOrder = c_ConstructionOrder(c);
  newOrder->m_type = type_id;
  newOrder->m_city_id = city_id;
  m_constructions[type] = id;
  return id;
}

bool ConstructionState::EraseConstruction(fbs::CONSTRUCTION_TYPE type_id) {
  uint32_t type = any_enum(type_id);
  return delete_c(m_constructions[type], s_ConstructionOrder()) != INVALID_COMPONENT;
}

bool ConstructionState::IsConstructed(fbs::CONSTRUCTION_TYPE type_id) const {
  if (!production::construction_is_unique(type_id)) {
    return false;
  }

  uint32_t type = any_enum(type_id);
  if (!m_constructions[type]) return false;

  uint32_t c = get(m_constructions[type], s_ConstructionOrder());
  return production::completed(c_ConstructionOrder(c));
}

ConstructionQueueFIFO::ConstructionQueueFIFO()
: m_city_id(0)
, m_stockpile(0.f)
{
  
}

std::ostream& operator<<(std::ostream& out, const ConstructionQueueFIFO& fifo) {
  out << "    Production: (stockpile " << fifo.m_stockpile << ")" << std::endl;
  out << *fifo.m_state;
  if (fifo.m_queue.empty()) return out;

  out << "    --Queued--" << std::endl;
  float turns = 0.0;
  for (auto co : fifo.m_queue) {
    turns += production::turns(co);
    out << "        ";
    out << production::name(co) << ": "
      << production::current(co)
      << "/" << production::required(co)
      << " (" << turns << " turns)";
    out << std::endl;
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const ConstructionState& state) {
  out << "    --Buildings--" << std::endl;
  for (auto construction : state.m_constructions) {
    if (!construction) continue;
    uint32_t c = get(construction, s_ConstructionOrder());
    ConstructionOrder* co = c_ConstructionOrder(c);
    if (!production::completed(co)) {
      continue;
    }
    out << "      " << EnumNameCONSTRUCTION_TYPE(co->m_type) << " is completed." << std::endl;
  }
  return out;
}


