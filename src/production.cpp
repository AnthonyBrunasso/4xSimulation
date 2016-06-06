
#include "production.h"

#include <iostream>
#include <iterator>

namespace city_production {
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
  return city_production::required(m_type_id) - m_production;
}

float ConstructionOrder::ApplyProduction(float production) {
  float required = city_production::required(m_type_id);
  
  float usedProduction = std::min(production, required - m_production);
  std::cout << "used production: " << usedProduction << std::endl;
  
  m_production += usedProduction;
  std::cout << "current production " << m_production << " / " << required << std::endl;
  return production - usedProduction;
}

const char* ConstructionOrder::GetName() {
  return city_production::name_of_construction(m_type_id);
}

bool ConstructionOrder::IsUnique() {
  return city_production::construction_is_unique(m_type_id);
}

bool ConstructionOrder::IsCompleted() {
  return m_production >= city_production::required(m_type_id);
}


ConstructionState::ConstructionState() {
}

ConstructionState::~ConstructionState() {
  for (auto construction : m_constructions) {
    delete construction.second;
  }
}

ConstructionOrder* ConstructionState::GetConstruction(CONSTRUCTION type_id) {
  if (!city_production::construction_is_unique(type_id)) {
    return new ConstructionOrder(type_id);
  }
  
  ConstructionUMap::const_iterator findIt = m_constructions.find(type_id);
  if (findIt != m_constructions.end()) {
    return findIt->second;
  }

  std::cout << "New unique construction: " << static_cast<int>(type_id) << std::endl;

  ConstructionOrder* newOrder = new ConstructionOrder(type_id);
  m_constructions.insert(findIt, ConstructionUMap::value_type(type_id, newOrder));
  
  return newOrder;
}

bool ConstructionState::IsConstructed(CONSTRUCTION type_id) {
  if (!city_production::construction_is_unique(type_id)) {
    return false;
  }

  ConstructionUMap::const_iterator itFind = m_constructions.find(type_id);
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
: m_production(0.f)
{
  
}

float ConstructionQueueFIFO::GetProductionYield() {
  float yield = 1.f;
  if (Has(CONSTRUCTION::FORGE)) {
    yield += 3.f;
  }

  return yield;
}
  
bool ConstructionQueueFIFO::Has(CONSTRUCTION type_id) {
  return m_state.IsConstructed(type_id);
}

void ConstructionQueueFIFO::Add(CONSTRUCTION type_id) {
  ConstructionOrder* order = m_state.GetConstruction(type_id);
  m_queue.push_back(order);
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

void ConstructionQueueFIFO::Simulation() {
  m_production += GetProductionYield();
  while (m_queue.size() > 0) {
    ConstructionOrder* order = m_queue.front();
    m_production = order->ApplyProduction(m_production);
    if (order->IsCompleted()) {
      ConstructionOrder* completed = m_queue.front();
      std::cout << "Construction completed: " << completed->GetName() << std::endl;
      m_queue.pop_front();
      if (!order->IsUnique()) {
        delete completed;
      }
    }
    if (m_production < 1.f) {
      return;
    }
  }
  
  // Limit the city's reserve when no construction orders are given
  //  The yield may have increased due to completed construction
  m_production = std::min(m_production, GetProductionYield());
}

void ConstructionQueueFIFO::PrintState() {
  std::cout << "--Construction State--" << std::endl;
  std::cout << "Stockpiled: " << m_production << std::endl;
  m_state.Print();
}

void ConstructionQueueFIFO::PrintQueue() {
  auto it = m_queue.cbegin();
  for (int i = 0; i < m_queue.size(); ++i, ++it) {
    std::cout << i << ") " << (*it)->GetName() << " remaining: " << (*it)->GetProductionForConstruction() << std::endl;
  }
}

