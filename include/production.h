
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <iosfwd>
#include <list>
#include <vector>

#include "game_types.h"

class ConstructionOrder;
class ConstructionQueueFIFO;
class ConstructionState;
struct TerrainYield;

typedef std::list<ConstructionOrder*> ConstructionList;

namespace production {
  typedef std::function<void(CONSTRUCTION_TYPE)> UnitCreationCallback;

  CONSTRUCTION_TYPE id(uint32_t);

  const char*name(ConstructionOrder*);
  float current(ConstructionOrder*);
  float remains(ConstructionOrder*);
  float required(ConstructionOrder*);
  float turns(ConstructionOrder*);
  float apply(ConstructionOrder*, float amount_available);
  bool completed(ConstructionOrder*);

  float required(CONSTRUCTION_TYPE type_id);
  float required_to_purchase(CONSTRUCTION_TYPE type_id);
  float yield_from_sale(CONSTRUCTION_TYPE type_id);
  bool construction_is_unique(CONSTRUCTION_TYPE type_id);

  uint32_t create(uint32_t city_id);
  ConstructionQueueFIFO* get_production(uint32_t production_id);
  void sub_create(const UnitCreationCallback&);

  void reset();
}

namespace production_queue {
  std::vector<CONSTRUCTION_TYPE> complete(const ConstructionQueueFIFO*);
  std::vector<CONSTRUCTION_TYPE> incomplete(const ConstructionQueueFIFO*);
  std::vector<CONSTRUCTION_TYPE> available(const ConstructionQueueFIFO*);
  std::vector<CONSTRUCTION_TYPE> queue(const ConstructionQueueFIFO*);
  
  CONSTRUCTION_TYPE front(const ConstructionQueueFIFO*);
  bool built(const ConstructionQueueFIFO*, CONSTRUCTION_TYPE);
  TerrainYield yield(const ConstructionQueueFIFO*);

  void add(ConstructionQueueFIFO*, CONSTRUCTION_TYPE);
  void move(ConstructionQueueFIFO*, size_t, size_t);
  void remove(ConstructionQueueFIFO*, size_t);
  void purchase(ConstructionQueueFIFO*, CONSTRUCTION_TYPE);
  void sell(ConstructionQueueFIFO*, CONSTRUCTION_TYPE);
  void simulate(ConstructionQueueFIFO*, TerrainYield& t);
}

class ConstructionQueueFIFO
{
public:
  explicit ConstructionQueueFIFO(uint32_t city_id);
  ConstructionQueueFIFO(ConstructionQueueFIFO&&) = default;

  ConstructionQueueFIFO(const ConstructionQueueFIFO&) = delete;
  ConstructionQueueFIFO& operator=(const ConstructionQueueFIFO&) = delete;

  uint32_t m_city_id;
  ConstructionList m_queue;
  ConstructionState* m_state;
  // When nothing is queued, the city can store limited production for future work
  float m_stockpile;
};

std::ostream& operator<<(std::ostream&, const ConstructionQueueFIFO&);

