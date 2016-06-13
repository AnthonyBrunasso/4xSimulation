
#include "production.h"
#include "city.h"
#include "player.h"
#include "units.h"
#include "terrain_yield.h"

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cmath>

namespace production {
  typedef std::vector<UnitCreationCallback> CreationCallbackVector;
  CreationCallbackVector s_creationCallbacks;

  void sub_create(const UnitCreationCallback& cb) {
    s_creationCallbacks.push_back(cb);
  }

  CONSTRUCTION_TYPE id(uint32_t type_id) {
    return static_cast<CONSTRUCTION_TYPE>(type_id);
  }

  float required(CONSTRUCTION_TYPE ) {
    return 60.f;
  }

  bool construction_is_unique(CONSTRUCTION_TYPE type_id) {
    return (static_cast<size_t>(type_id) & 1) != 0;
  }

  void spawn_unit(Player& player, CONSTRUCTION_TYPE type_id, City* city) {
    if (!player.OwnsCity(city->m_id)) {
      return;
    }

    uint32_t unit_id;
    switch (type_id) {
    case CONSTRUCTION_TYPE::SCOUT:
      unit_id = units::create(UNIT_TYPE::SCOUT, city->m_location);
      break;
    case CONSTRUCTION_TYPE::RANGE:
      unit_id = units::create(UNIT_TYPE::ARCHER, city->m_location);
      break;
    case CONSTRUCTION_TYPE::MELEE:
      unit_id = units::create(UNIT_TYPE::PHALANX, city->m_location);
      break;
    case CONSTRUCTION_TYPE::WORKER:
      unit_id = units::create(UNIT_TYPE::WORKER, city->m_location);
      break;
    default:
      return;
    }
    player::add_unit(player.m_id, unit_id);
  }
}

ConstructionOrder::ConstructionOrder(CONSTRUCTION_TYPE type_id)
: m_type_id(type_id)
, m_production(0.f)
{
}

float ConstructionOrder::GetProductionForConstruction() {
  return production::required(m_type_id) - m_production;
}

float ConstructionOrder::ApplyProduction(float production) {
  float required = production::required(m_type_id);
  
  float usedProduction = std::min(production, required - m_production);
  //std::cout << "used production: " << usedProduction << std::endl;
  
  m_production += usedProduction;
  //std::cout << "current production " << m_production << " / " << required << std::endl;
  return production - usedProduction;
}

std::string ConstructionOrder::GetName() {
  return std::move(get_construction_name(m_type_id));
}

bool ConstructionOrder::IsUnique() {
  return production::construction_is_unique(m_type_id);
}

bool ConstructionOrder::IsCompleted() {
  return m_production >= production::required(m_type_id);
}

CONSTRUCTION_TYPE ConstructionOrder::GetType() {
  return m_type_id;
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
    return findIt->second;
  }

  std::cout << "New unique construction: " << get_construction_name(type_id) << std::endl;

  ConstructionOrder* newOrder = new ConstructionOrder(type_id);
  m_constructions.insert(findIt, ConstructionUMap::value_type(type, newOrder));
  
  return newOrder;
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

  return itFind->second->IsCompleted();
}

ConstructionQueueFIFO::ConstructionQueueFIFO(uint32_t cityId)
: m_cityId(cityId)
, m_stockpile(0.f)
{
  
}

bool ConstructionQueueFIFO::Has(CONSTRUCTION_TYPE type_id) const {
  return m_state.IsConstructed(type_id);
}

void ConstructionQueueFIFO::Add(CONSTRUCTION_TYPE type_id) {
  if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
    return;
  }

  ConstructionOrder* order = m_state.GetConstruction(type_id);
  m_queue.push_back(order);
}

void ConstructionQueueFIFO::Cheat(CONSTRUCTION_TYPE type_id) {
  if (type_id == CONSTRUCTION_TYPE::UNKNOWN) {
    return;
  }

  if (!production::construction_is_unique(type_id)) {
    return;
  }

  ConstructionOrder* order = m_state.GetConstruction(type_id);
  order->ApplyProduction(9999.f);
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

  auto itFrom = m_queue.begin();
  std::advance(itFrom, src);
  ConstructionOrder* order = *itFrom;
  m_queue.erase(itFrom);

  auto itTo = m_queue.begin();
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
    yield += 3.f;
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
    if (order->IsCompleted()) {
      ConstructionOrder* completed = m_queue.front();
      std::cout << "Construction completed: " << completed->GetName() << std::endl;
      m_queue.pop_front();
      if (!order->IsUnique()) {
        // Unit spawn
        player::for_each_player(std::bind(&production::spawn_unit, 
          std::placeholders::_1,
          completed->GetType(),
          parent));
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
  for (size_t i = 0; i < fifo.m_queue.size(); ++i, ++it) {
    out << "        ";
    out << i << ") " << (*it)->GetName() << " remaining: " << (*it)->GetProductionForConstruction() 
        << " (" << ceil((*it)->GetProductionForConstruction()/t.m_production) << " turns)";
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
    if (!construction.second->IsCompleted()) {
      continue;
    }
    out << "      " << construction.second->GetName() << " is completed." << std::endl;
  }
  return out;
}


