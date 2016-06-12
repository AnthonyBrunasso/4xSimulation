
#pragma once

#include <unordered_map>
#include <list>
#include <functional>
#include <string>

#include "game_types.h"

class City;
struct TerrainYield;

namespace production {
  typedef std::function<void (CONSTRUCTION_TYPE)> UnitCreationCallback;

  CONSTRUCTION_TYPE id(uint32_t);
  float required(CONSTRUCTION_TYPE type_id);
  bool construction_is_unique(CONSTRUCTION_TYPE type_id);

  void sub_create(const UnitCreationCallback&);
}

class ConstructionOrder;

typedef std::unordered_map<uint32_t, ConstructionOrder*> ConstructionUMap;
typedef std::list<ConstructionOrder*> ConstructionList;

class ConstructionOrder
{
public:
  ConstructionOrder(CONSTRUCTION_TYPE type_id);

  float GetProductionForConstruction();
  float ApplyProduction(float production);

  std::string GetName();
  bool IsUnique();
  bool IsCompleted();
  CONSTRUCTION_TYPE GetType();

private:
  ConstructionOrder();

  CONSTRUCTION_TYPE m_type_id;
  float m_production;
};

class ConstructionState
{
public:
  ConstructionState();
  ~ConstructionState();

  ConstructionOrder* GetConstruction(CONSTRUCTION_TYPE type_id);
  bool IsConstructed(CONSTRUCTION_TYPE type_id) const;
  void Print() const;

private:
  ConstructionUMap m_constructions;
};


class ConstructionQueueFIFO
{
public:
  ConstructionQueueFIFO();

  bool Has(CONSTRUCTION_TYPE type_id) const;
  void Add(CONSTRUCTION_TYPE type_id);
  void Cheat(CONSTRUCTION_TYPE type_id);

  void Move(size_t src, size_t dest);
  size_t Count() const;

  void MutateYield(TerrainYield&) const;
  void Simulate(City* parent, TerrainYield&);

  void PrintState() const;
  void PrintQueue(TerrainYield&) const;
  
private:
  ConstructionList m_queue;
  ConstructionState m_state;
  // When nothing is queued, the city can store limited production for future work
  float m_stockpile;
};

