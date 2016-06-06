
#include "production.h"

#include <iostream>
#include <iterator>

namespace production {
  CONSTRUCTION id(uint32_t type_id) {
    if (type_id <= static_cast<uint32_t>(CONSTRUCTION::INVALID)) {
      return static_cast<CONSTRUCTION>(type_id);
    }
   
    return CONSTRUCTION::INVALID;
  }

  float required(CONSTRUCTION ) {
    return 60.f;
  }

  const char* name_of_construction(CONSTRUCTION id) {
    const char *names[] = {
     "Scout",
     "Granary",
     "Archer",
     "Forge",
     "Phalanx" };
    
    return names[static_cast<int>(id)];
  }

  bool construction_is_unique(CONSTRUCTION type_id) {
    return (static_cast<size_t>(type_id) & 1) != 0;
  }
}

ConstructionOrder::ConstructionOrder(CONSTRUCTION type_id)
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
  std::cout << "used production: " << usedProduction << std::endl;
  
  m_production += usedProduction;
  std::cout << "current production " << m_production << " / " << required << std::endl;
  return production - usedProduction;
}

const char* ConstructionOrder::GetName() {
  return production::name_of_construction(m_type_id);
}

bool ConstructionOrder::IsUnique() {
  return production::construction_is_unique(m_type_id);
}

bool ConstructionOrder::IsCompleted() {
  return m_production >= production::required(m_type_id);
}


ConstructionState::ConstructionState() {
}

ConstructionState::~ConstructionState() {
  for (auto construction : m_constructions) {
    delete construction.second;
  }
}

ConstructionOrder* ConstructionState::GetConstruction(CONSTRUCTION type_id) {
  if (!production::construction_is_unique(type_id)) {
    return new ConstructionOrder(type_id);
  }
  
  uint32_t type = static_cast<uint32_t>(type_id);
  ConstructionUMap::const_iterator findIt = m_constructions.find(type);
  if (findIt != m_constructions.end()) {
    return findIt->second;
  }

  std::cout << "New unique construction: " << type << std::endl;

  ConstructionOrder* newOrder = new ConstructionOrder(type_id);
  m_constructions.insert(findIt, ConstructionUMap::value_type(type, newOrder));
  
  return newOrder;
}

bool ConstructionState::IsConstructed(CONSTRUCTION type_id) {
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

void ConstructionState::Print() {
  for (auto construction : m_constructions) {
    std::cout << " " << construction.second->GetName() << (construction.second->IsCompleted()? " is completed.":" is in progress.") << std::endl;
  }
}

ConstructionQueueFIFO::ConstructionQueueFIFO()
: m_stockpile(0.f)
{
  
}

float ConstructionQueueFIFO::GetProductionYield() {
  float yield = 1.f;
  if (Has(CONSTRUCTION::FORGE)) {
    yield += 3.f;
  }
  if (Has(CONSTRUCTION::UBER_FORGE)) {
    yield += 10.0f;
  }

  return yield;
}
  
bool ConstructionQueueFIFO::Has(CONSTRUCTION type_id) {
  return m_state.IsConstructed(type_id);
}

void ConstructionQueueFIFO::Add(CONSTRUCTION type_id) {
  if (type_id == CONSTRUCTION::INVALID) {
    return;
  }

  ConstructionOrder* order = m_state.GetConstruction(type_id);
  m_queue.push_back(order);
}

void ConstructionQueueFIFO::Cheat(CONSTRUCTION type_id) {
  if (type_id == CONSTRUCTION::INVALID) {
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

void ConstructionQueueFIFO::Simulate() {
  m_stockpile += GetProductionYield();
  while (m_queue.size() > 0) {
    ConstructionOrder* order = m_queue.front();
    m_stockpile = order->ApplyProduction(m_stockpile);
    if (order->IsCompleted()) {
      ConstructionOrder* completed = m_queue.front();
      std::cout << "Construction completed: " << completed->GetName() << std::endl;
      m_queue.pop_front();
      if (!order->IsUnique()) {
        delete completed;
      }
    }
    if (m_stockpile < 1.f) {
      return;
    }
  }
  
  // Limit the city's reserve when no construction orders are given
  //  The yield may have increased due to completed construction
  m_stockpile = std::min(m_stockpile, GetProductionYield());
}

void ConstructionQueueFIFO::PrintState() {
  std::cout << "--Construction State--" << std::endl;
  std::cout << "Stockpiled: " << m_stockpile << std::endl;
  m_state.Print();
}

void ConstructionQueueFIFO::PrintQueue() {
  auto it = m_queue.cbegin();
  for (size_t i = 0; i < m_queue.size(); ++i, ++it) {
    std::cout << i << ") " << (*it)->GetName() << " remaining: " << (*it)->GetProductionForConstruction() << std::endl;
  }
}

