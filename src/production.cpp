
#include "production.h"
#include "production_detail.h"
#include "city.h"
#include "player.h"
#include "unit.h"
#include "terrain_yield.h"
#include "unique_id.h"

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cmath>

typedef std::unordered_map<uint32_t, ConstructionOrder*> ConstructionUMap;
class ConstructionState
{
public:
  ConstructionState();
  ~ConstructionState();
  ConstructionState(ConstructionState&&) = default;

  ConstructionState(const ConstructionState&) = delete;
  ConstructionState& operator=(const ConstructionState&) = delete;

  ConstructionOrder* GetConstruction(CONSTRUCTION_TYPE type_id, uint32_t city_id);
  bool EraseConstruction(CONSTRUCTION_TYPE type_id);
  bool IsConstructed(CONSTRUCTION_TYPE type_id) const;

private:
  friend std::ostream& operator<<(std::ostream&, const ConstructionState&);

  ConstructionUMap m_constructions;
};
std::ostream& operator<<(std::ostream&, const ConstructionState&);

namespace production {
  typedef std::vector<UnitCreationCallback> CreationCallbackVector;
  CreationCallbackVector s_creationCallbacks;
  typedef std::unordered_map<uint32_t, ConstructionQueueFIFO*> ProductionMap;
  ProductionMap s_production_queues;

  void production_cleanup(const sf::Vector3i&, uint32_t city_id) {
    City* c = city::get_city(city_id);
    if (!c) return;

    ProductionMap::iterator itFind = s_production_queues.find(c->m_production_id);
    if (itFind == s_production_queues.end()) {
      return;
    }
    
    ConstructionQueueFIFO* cq = itFind->second;
    delete cq->m_state;
    delete cq;
    s_production_queues.erase(itFind);
  }
  
  uint32_t create(uint32_t city_id) {
    uint32_t queue_id = unique_id::generate();
    ConstructionQueueFIFO* cq = new ConstructionQueueFIFO(city_id);
    cq->m_state = new ConstructionState();
    s_production_queues[queue_id] = cq;
    city::sub_raze_complete(production_cleanup);
    return queue_id;
  }

  ConstructionQueueFIFO* get_production(uint32_t production_id) {
    ProductionMap::const_iterator itFind = s_production_queues.find(production_id);

    if (itFind == s_production_queues.end()) {
      return nullptr;
    }

    return itFind->second;
  }

  void sub_create(const UnitCreationCallback& cb) {
    s_creationCallbacks.push_back(cb);
  }

  CONSTRUCTION_TYPE id(uint32_t type_id) {
    return static_cast<CONSTRUCTION_TYPE>(type_id);
  }

  const char*name(ConstructionOrder* co) {
    return get_construction_name(co->m_type);
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

  float required(CONSTRUCTION_TYPE type) {
    switch (type) {
    case CONSTRUCTION_TYPE::GRANARY:
      return 40.f;
    case CONSTRUCTION_TYPE::RANGE :
      return 30.f;
    case CONSTRUCTION_TYPE::FORGE :
      return 12.f;
    case CONSTRUCTION_TYPE::MELEE :
      return 30.f;
    case CONSTRUCTION_TYPE::FACTORY :
      return 60.f;
    case CONSTRUCTION_TYPE::SCOUT :
      return 10.f;
    case CONSTRUCTION_TYPE::WORKER :
      return 20.f;
    case CONSTRUCTION_TYPE::UNKNOWN :
    default:
      return 1000.f;
    }
  }

  float required_to_purchase(CONSTRUCTION_TYPE type) {
    return required(type) * 3.f;
  }
  
  float yield_from_sale(CONSTRUCTION_TYPE type) {
    return required_to_purchase(type) * .25f;
  }

  bool construction_is_unique(CONSTRUCTION_TYPE type_id) {
    return (static_cast<size_t>(type_id) & 1) != 0;
  }

  void spawn_unit(CONSTRUCTION_TYPE type_id, uint32_t city_id) {
    City* city = city::get_city(city_id);
    if (!city) return;

    uint32_t unit_id;
    switch (type_id) {
    case CONSTRUCTION_TYPE::SCOUT:
      unit_id = unit::create(UNIT_TYPE::SCOUT, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::RANGE:
      unit_id = unit::create(UNIT_TYPE::ARCHER, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::MELEE:
      unit_id = unit::create(UNIT_TYPE::PHALANX, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::WORKER:
      unit_id = unit::create(UNIT_TYPE::WORKER, city->m_location, city->m_owner_id);
      break;
    default:
      return;
    }
    player::add_unit(city->m_owner_id, unit_id);
  }
}

namespace production_queue {
  std::vector<CONSTRUCTION_TYPE> complete(const ConstructionQueueFIFO* cq) {
    std::vector<CONSTRUCTION_TYPE> constructed;
    for_each_construction_type([cq, &constructed](CONSTRUCTION_TYPE t) {
      if (!cq->m_state->IsConstructed(t)) return;
      constructed.push_back(t);
    });
    return std::move(constructed);
  }

  std::vector<CONSTRUCTION_TYPE> incomplete(const ConstructionQueueFIFO* cq) {
    std::vector<CONSTRUCTION_TYPE> incomplete;
    for_each_construction_type([cq, &incomplete](CONSTRUCTION_TYPE t) {
      if (cq->m_state->IsConstructed(t)) return;
      incomplete.push_back(t);
    });
    return std::move(incomplete);
  }

  std::vector<CONSTRUCTION_TYPE> queue(const ConstructionQueueFIFO* cq) {
    std::vector<CONSTRUCTION_TYPE> queue;
    for (auto& q : cq->m_queue) {
      queue.push_back(q->m_type);
    }

    return std::move(queue);
  }

  CONSTRUCTION_TYPE front(const ConstructionQueueFIFO* cq) {
    if (cq->m_queue.empty()) return CONSTRUCTION_TYPE::UNKNOWN;
    return cq->m_queue.front()->m_type;
  }

  bool built(const ConstructionQueueFIFO* cq, CONSTRUCTION_TYPE type_id) {
    return cq->m_state->IsConstructed(type_id);
  }

  TerrainYield yield(const ConstructionQueueFIFO* cq) {
    TerrainYield ty;
    ty.m_production = 1.f;

    if (built(cq, CONSTRUCTION_TYPE::FORGE)) {
      ty.m_production += 1.5f;
    }
    if (built(cq, CONSTRUCTION_TYPE::FACTORY)) {
      ty.m_production += 10.0f;
    }
    if (built(cq, CONSTRUCTION_TYPE::GRANARY)) {
      ty.m_food += 2.0f;
    }

    return ty;
  }

  void add(ConstructionQueueFIFO* cq, CONSTRUCTION_TYPE type_id) {
    if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
      std::cout << "Construction Add on unknown type" << std::endl;
      return;
    }

    ConstructionOrder* order = cq->m_state->GetConstruction(type_id, cq->m_city_id);
    cq->m_queue.push_back(order);
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
    ConstructionOrder* order = *itFrom;
    cq->m_queue.erase(itFrom);

    std::list<ConstructionOrder*>::iterator itTo = cq->m_queue.begin();
    std::advance(itTo, dest);
    cq->m_queue.insert(itTo, order);
  }

  void remove(ConstructionQueueFIFO* cq, size_t offset) {
    if (offset >= cq->m_queue.size()) return;
    std::list<ConstructionOrder*>::iterator itr = cq->m_queue.begin();
    std::advance(itr, offset);
    cq->m_queue.erase(itr);
  }

  void purchase(ConstructionQueueFIFO* cq, CONSTRUCTION_TYPE type_id) {
    if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
      std::cout << "Construction purchase on unknown type" << std::endl;
      return;
    }
    
    if (!production::construction_is_unique(type_id)) {
      production::spawn_unit(type_id, cq->m_city_id);
      return;
    }

    ConstructionOrder* order = cq->m_state->GetConstruction(type_id, cq->m_city_id);
    production::apply(order, 9999.f);
  }

  void sell(ConstructionQueueFIFO* cq, CONSTRUCTION_TYPE type_id) {
    if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
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
        std::cout << "Construction completed: " << get_construction_name(completed->m_type) << std::endl;
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
}

ConstructionState::~ConstructionState() {
  for (auto construction : m_constructions) {
    delete construction.second;
  }
}

ConstructionOrder* ConstructionState::GetConstruction(CONSTRUCTION_TYPE type_id, uint32_t city_id) {
  if (!production::construction_is_unique(type_id)) {
    return new ConstructionOrder(type_id, city_id);
  }
  
  uint32_t type = static_cast<uint32_t>(type_id);
  ConstructionUMap::const_iterator findIt = m_constructions.find(type);
  if (findIt != m_constructions.end()) {
    std::cout << "Resuming unique construction: " << get_construction_name(type_id) << std::endl;
    return findIt->second;
  }

  std::cout << "New unique construction: " << get_construction_name(type_id) << std::endl;

  ConstructionOrder* newOrder = new ConstructionOrder(type_id, city_id);
  m_constructions.insert(findIt, ConstructionUMap::value_type(type, newOrder));
  
  return newOrder;
}

bool ConstructionState::EraseConstruction(CONSTRUCTION_TYPE type_id) {
  return m_constructions.erase(static_cast<uint32_t>(type_id)) != 0;
}

bool ConstructionState::IsConstructed(CONSTRUCTION_TYPE type_id) const {
  if (!production::construction_is_unique(type_id)) {
    return false;
  }

  uint32_t type = static_cast<uint32_t>(type_id);
  ConstructionUMap::const_iterator itFind = m_constructions.find(type);
  if (itFind == m_constructions.end()) {
    return false;
  }

  return production::completed(itFind->second);
}

ConstructionQueueFIFO::ConstructionQueueFIFO(uint32_t city_id)
: m_city_id(city_id)
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
  if (state.m_constructions.empty()) {
    return out;
  }
  out << "    --Buildings--" << std::endl;
  for (auto construction : state.m_constructions) {
    if (!production::completed(construction.second)) {
      continue;
    }
    out << "      " << get_construction_name(construction.second->m_type) << " is completed." << std::endl;
  }
  return out;
}


