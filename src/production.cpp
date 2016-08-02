
#include "production.h"
#include "city.h"
#include "player.h"
#include "units.h"
#include "terrain_yield.h"
#include "unique_id.h"

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cmath>

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
    
    delete itFind->second;
    s_production_queues.erase(itFind);
  }
  
  uint32_t create(uint32_t city_id) {
    uint32_t queue_id = unique_id::generate();
    s_production_queues[queue_id] = new ConstructionQueueFIFO(city_id);
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

  float remains(ConstructionOrder* order) {
    return required(order->m_type) - order->m_production;
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

  void spawn_unit(CONSTRUCTION_TYPE type_id, City* city) {
    uint32_t unit_id;
    switch (type_id) {
    case CONSTRUCTION_TYPE::SCOUT:
      unit_id = units::create(UNIT_TYPE::SCOUT, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::RANGE:
      unit_id = units::create(UNIT_TYPE::ARCHER, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::MELEE:
      unit_id = units::create(UNIT_TYPE::PHALANX, city->m_location, city->m_owner_id);
      break;
    case CONSTRUCTION_TYPE::WORKER:
      unit_id = units::create(UNIT_TYPE::WORKER, city->m_location, city->m_owner_id);
      break;
    default:
      return;
    }
    player::add_unit(city->m_owner_id, unit_id);
  }
}

ConstructionOrder::ConstructionOrder(CONSTRUCTION_TYPE type_id)
: m_type(type_id)
, m_production(0.f)
{
}

float ConstructionOrder::ApplyProduction(float production) {
  float required = production::required(m_type);
  
  float usedProduction = std::min(production, required - m_production);
  //std::cout << "used production: " << usedProduction << std::endl;
  
  m_production += usedProduction;
  //std::cout << "current production " << m_production << " / " << required << std::endl;
  return production - usedProduction;
}

bool ConstructionOrder::IsComplete() {
  return m_production >= production::required(m_type);
}

ConstructionState::ConstructionState() {
}

ConstructionState::~ConstructionState() {
  for (auto construction : m_constructions) {
    delete construction.second;
  }
}

ConstructionOrder* ConstructionState::GetConstruction(CONSTRUCTION_TYPE type_id) {
  if (!production::construction_is_unique(type_id)) {
    return new ConstructionOrder(type_id);
  }
  
  uint32_t type = static_cast<uint32_t>(type_id);
  ConstructionUMap::const_iterator findIt = m_constructions.find(type);
  if (findIt != m_constructions.end()) {
    std::cout << "Resuming unique construction: " << get_construction_name(type_id) << std::endl;
    return findIt->second;
  }

  std::cout << "New unique construction: " << get_construction_name(type_id) << std::endl;

  ConstructionOrder* newOrder = new ConstructionOrder(type_id);
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

  return itFind->second->IsComplete();
}

std::vector<CONSTRUCTION_TYPE> ConstructionState::GetComplete() const {
  std::vector<CONSTRUCTION_TYPE> constructed;
  for_each_construction_type([this, &constructed] (CONSTRUCTION_TYPE t) {
    if (!IsConstructed(t)) return;
    constructed.push_back(t);
  });
  return std::move(constructed);
}

std::vector<CONSTRUCTION_TYPE> ConstructionState::GetIncomplete() const {
  std::vector<CONSTRUCTION_TYPE> incomplete;
  for_each_construction_type([this, &incomplete] (CONSTRUCTION_TYPE t) {
    if (IsConstructed(t)) return;
    incomplete.push_back(t);
  });
  return std::move(incomplete);
}

ConstructionQueueFIFO::ConstructionQueueFIFO(uint32_t cityId)
: m_cityId(cityId)
, m_stockpile(0.f)
{
  
}

std::vector<CONSTRUCTION_TYPE> ConstructionQueueFIFO::Complete() const {
  return std::move(m_state.GetComplete());
}

std::vector<CONSTRUCTION_TYPE> ConstructionQueueFIFO::Incomplete() const {
  return std::move(m_state.GetIncomplete());
}

std::vector<CONSTRUCTION_TYPE> ConstructionQueueFIFO::Queue() const {
  std::vector<CONSTRUCTION_TYPE> queue;
  for (auto& q : m_queue) {
    queue.push_back(q->m_type);
  }

  return std::move(queue);
}

bool ConstructionQueueFIFO::Has(CONSTRUCTION_TYPE type_id) const {
  return m_state.IsConstructed(type_id);
}

void ConstructionQueueFIFO::Add(CONSTRUCTION_TYPE type_id) {
  if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
    std::cout << "Construction Add on unknown type" << std::endl;
    return;
  }

  ConstructionOrder* order = m_state.GetConstruction(type_id);
  m_queue.push_back(order);
}

void ConstructionQueueFIFO::Purchase(CONSTRUCTION_TYPE type_id, City* parent) {
  if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
    std::cout << "Construction purchase on unknown type" << std::endl;
    return;
  }

  if (!production::construction_is_unique(type_id)) {
    production::spawn_unit(type_id, parent);
    return;
  }

  ConstructionOrder* order = m_state.GetConstruction(type_id);
  order->ApplyProduction(9999.f);
}

void ConstructionQueueFIFO::Sell(CONSTRUCTION_TYPE type_id) {
  if (type_id == CONSTRUCTION_TYPE::UNKNOWN) { 
    std::cout << "Construction sale on unknown type" << std::endl;
    return;
  }
  
  if (!production::construction_is_unique(type_id)) {
    return;
  }

  m_state.EraseConstruction(type_id);
}

CONSTRUCTION_TYPE ConstructionQueueFIFO::Current() {
  if (m_queue.empty()) return CONSTRUCTION_TYPE::UNKNOWN;
  return m_queue.front()->m_type;
}

void ConstructionQueueFIFO::Abort(size_t offset) {
  if (offset >= m_queue.size()) return;
  std::list<ConstructionOrder*>::iterator itr = m_queue.begin();
  std::advance(itr, offset);
  m_queue.erase(itr);
}

void ConstructionQueueFIFO::Move(size_t src, size_t dest) { 
  if (src >= m_queue.size()) {
    std::cout << "Invalid source index, list swap aborted" << std::endl;
    return;
  }

  if (dest >= m_queue.size()) {
    std::cout << "Invalid destination index, list swap aborted" << std::endl;
    return;
  }

  std::list<ConstructionOrder*>::iterator itFrom = m_queue.begin();
  std::advance(itFrom, src);
  ConstructionOrder* order = *itFrom;
  m_queue.erase(itFrom);

  std::list<ConstructionOrder*>::iterator itTo = m_queue.begin();
  std::advance(itTo, dest);
  m_queue.insert(itTo, order);
}

size_t ConstructionQueueFIFO::Count() const {
  return m_queue.size();
}

TerrainYield ConstructionQueueFIFO::DumpYields() const {
  City* city = city::get_city(m_cityId);
  return city->DumpYields();
}

void ConstructionQueueFIFO::MutateYield(TerrainYield& t) const {
 float yield = 1.f;
  if (Has(CONSTRUCTION_TYPE::FORGE)) {
    yield += 1.5f;
  }
  if (Has(CONSTRUCTION_TYPE::FACTORY)) {
    yield += 10.0f;
  }
  t.m_production += yield;
}

void ConstructionQueueFIFO::Simulate(City* parent, TerrainYield& t) {
  m_stockpile += t.m_production;
  while (m_queue.size() > 0) {
    ConstructionOrder* order = m_queue.front();
    m_stockpile = order->ApplyProduction(m_stockpile);
    if (order->IsComplete()) {
      ConstructionOrder* completed = m_queue.front();
      std::cout << "Construction completed: " << get_construction_name(completed->m_type) << std::endl;
      m_queue.pop_front();
      if (!production::construction_is_unique(order->m_type)) {
        // Unit spawn
        production::spawn_unit(completed->m_type, parent);
        delete completed;
      }
    }
    if (m_stockpile < 1.f) {
      return;
    }
  }
  
  // Limit the city's reserve when no construction orders are given
  //  The yield may have increased due to completed construction
  m_stockpile = std::min(m_stockpile, t.m_production);
}

std::ostream& operator<<(std::ostream& out, const ConstructionQueueFIFO& fifo) {
  out << "    Production: (stockpile " << fifo.m_stockpile << ")" << std::endl;
  out << fifo.m_state;
  if (fifo.m_queue.empty()) {
    return out;
  }
  TerrainYield t = fifo.DumpYields();
  out << "    --Queued--" << std::endl;
  auto it = fifo.m_queue.cbegin();
  uint32_t turns = 0;
  for (size_t i = 0; i < fifo.m_queue.size(); ++i, ++it) {
    turns += static_cast<uint32_t>(ceil((production::required((*it)->m_type)-(*it)->m_production)/t.m_production));
    out << "        ";
    out << i << ") " << get_construction_name((*it)->m_type) << ": " << (*it)->m_production << "/" << production::required((*it)->m_type)
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
    if (!construction.second->IsComplete()) {
      continue;
    }
    out << "      " << get_construction_name(construction.second->m_type) << " is completed." << std::endl;
  }
  return out;
}


