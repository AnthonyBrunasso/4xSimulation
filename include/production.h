
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <iosfwd>
#include <list>
#include <vector>

#include "enum_generated.h"


class ConstructionOrder;
class ConstructionQueueFIFO;
class ConstructionState;
struct TerrainYield;

typedef std::list<ConstructionOrder*> ConstructionList;

namespace production {
  fbs::CONSTRUCTION_TYPE id(uint32_t);

  const char*name(ConstructionOrder*);
  float current(ConstructionOrder*);
  float remains(ConstructionOrder*);
  float required(ConstructionOrder*);
  float turns(ConstructionOrder*);
  float apply(ConstructionOrder*, float amount_available);
  bool completed(ConstructionOrder*);

  float required(fbs::CONSTRUCTION_TYPE type_id);
  float required_to_purchase(fbs::CONSTRUCTION_TYPE type_id);
  float yield_from_sale(fbs::CONSTRUCTION_TYPE type_id);
  bool construction_is_unique(fbs::CONSTRUCTION_TYPE type_id);

  uint32_t create(uint32_t city_id);
  ConstructionQueueFIFO* get_production(uint32_t production_id);

  void reset();
}

namespace production_queue {
  std::vector<uint32_t> complete(const ConstructionQueueFIFO*);
  std::vector<uint32_t> incomplete(const ConstructionQueueFIFO*);
  std::vector<uint32_t> available(const ConstructionQueueFIFO*);
  std::vector<uint32_t> queue(const ConstructionQueueFIFO*);
  
  fbs::CONSTRUCTION_TYPE front(const ConstructionQueueFIFO*);
  bool built(const ConstructionQueueFIFO*, fbs::CONSTRUCTION_TYPE);
  TerrainYield yield(const ConstructionQueueFIFO*);

  void add(ConstructionQueueFIFO*, fbs::CONSTRUCTION_TYPE);
  void move(ConstructionQueueFIFO*, size_t, size_t);
  void remove(ConstructionQueueFIFO*, size_t);
  void purchase(ConstructionQueueFIFO*, fbs::CONSTRUCTION_TYPE);
  void sell(ConstructionQueueFIFO*, fbs::CONSTRUCTION_TYPE);
  void simulate(ConstructionQueueFIFO*, TerrainYield& t);
}

class ConstructionQueueFIFO
{
public:
  ConstructionQueueFIFO();
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

