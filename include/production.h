
#pragma once

#include <unordered_map>
#include <list>
#include <functional>
#include <vector>
#include <string>

#include "game_types.h"

class City;
struct TerrainYield;

namespace production {
  typedef std::function<void (CONSTRUCTION_TYPE)> UnitCreationCallback;

  CONSTRUCTION_TYPE id(uint32_t);
  float required(CONSTRUCTION_TYPE type_id);
  float required_to_purchase(CONSTRUCTION_TYPE type_id);
  float yield_from_sale(CONSTRUCTION_TYPE type_id);
  bool construction_is_unique(CONSTRUCTION_TYPE type_id);

  void sub_create(const UnitCreationCallback&);
}

class ConstructionOrder;

typedef std::unordered_map<uint32_t, ConstructionOrder*> ConstructionUMap;
typedef std::list<ConstructionOrder*> ConstructionList;

class ConstructionOrder
{
public:
  explicit ConstructionOrder(CONSTRUCTION_TYPE type_id);

  float GetProductionForConstruction();
  float ApplyProduction(float production);

  std::string GetName();
  bool IsUnique();
  bool IsComplete();
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
  ConstructionState(ConstructionState&&) = default;

  ConstructionState(const ConstructionState&) = delete;
  ConstructionState& operator=(const ConstructionState&) = delete;  

  ConstructionOrder* GetConstruction(CONSTRUCTION_TYPE type_id);
  bool EraseConstruction(CONSTRUCTION_TYPE type_id);
  bool IsConstructed(CONSTRUCTION_TYPE type_id) const;
  std::vector<CONSTRUCTION_TYPE> GetComplete() const;
  std::vector<CONSTRUCTION_TYPE> GetIncomplete() const;

private:
  friend std::ostream& operator<<(std::ostream&, const ConstructionState&);

  ConstructionUMap m_constructions;
};


class ConstructionQueueFIFO
{
public:
  explicit ConstructionQueueFIFO(uint32_t cityId);
  ConstructionQueueFIFO(ConstructionQueueFIFO&&) = default;

  ConstructionQueueFIFO(const ConstructionQueueFIFO&) = delete;
  ConstructionQueueFIFO& operator=(const ConstructionQueueFIFO&) = delete;

  std::vector<CONSTRUCTION_TYPE> Complete() const;
  std::vector<CONSTRUCTION_TYPE> Incomplete() const;
  bool Has(CONSTRUCTION_TYPE type_id) const;
  void Add(CONSTRUCTION_TYPE type_id);
  void Purchase(CONSTRUCTION_TYPE type_id, City* parent);
  void Sell(CONSTRUCTION_TYPE type_id);

  void Abort(size_t offset);
  void Move(size_t src, size_t dest);
  size_t Count() const;

  TerrainYield DumpYields() const;
  void MutateYield(TerrainYield&) const;
  void Simulate(City* parent, TerrainYield&);

private:
  friend std::ostream& operator<<(std::ostream&, const ConstructionQueueFIFO&);

  uint32_t m_cityId;
  ConstructionList m_queue;
  ConstructionState m_state;
  // When nothing is queued, the city can store limited production for future work
  float m_stockpile;
};

std::ostream& operator<<(std::ostream&, const ConstructionQueueFIFO&);
std::ostream& operator<<(std::ostream&, const ConstructionState&);

