
#pragma once

#include <unordered_map>
#include <list>

enum class CONSTRUCTION
{
  SCOUT_UNIT = 0,
  GRANARY,
  RANGE_UNIT,
  FORGE,
  MELEE_UNIT,
};

namespace std {
  template <>
  struct hash<CONSTRUCTION> {
    std::size_t operator()(const CONSTRUCTION& type_id) const {
      return static_cast<size_t>(type_id);
    }
  };
}


namespace production {
  float required(CONSTRUCTION type_id);
  const char* name_of_construction(CONSTRUCTION type_id);
  bool construction_is_unique(CONSTRUCTION type_id);
}

class ConstructionOrder;

typedef std::unordered_map<CONSTRUCTION, ConstructionOrder*> ConstructionUMap;
typedef std::list<ConstructionOrder*> ConstructionList;

class ConstructionOrder
{
public:
  ConstructionOrder(CONSTRUCTION type_id);

  float GetProductionForConstruction();
  float ApplyProduction(float production);

  const char* GetName();
  bool IsUnique();
  bool IsCompleted();

private:
  ConstructionOrder();

  CONSTRUCTION m_type_id;
  float m_production;
};

class ConstructionState
{
public:
  ConstructionState();
  ~ConstructionState();

  ConstructionOrder* GetConstruction(CONSTRUCTION type_id);
  bool IsConstructed(CONSTRUCTION type_id);
  void Print();

private:
  ConstructionUMap m_constructions;
};


class ConstructionQueueFIFO
{
public:
  ConstructionQueueFIFO();

  float GetProductionYield();

  bool Has(CONSTRUCTION type_id);
  void Add(CONSTRUCTION type_id);

  void Move(size_t src, size_t dest);

  void Simulation();

  void PrintState();
  void PrintQueue();
  
private:
  ConstructionList m_queue;
  ConstructionState m_state;
  // When nothing is queued, the city can store limited production for future work
  float m_production;
};

