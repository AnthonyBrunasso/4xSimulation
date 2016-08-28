#pragma once

#include "dtree.h"

class EmpireColonize : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class EmpireProduce : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class EmpireUnits : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

// Checks if any cities have been discover.
class EmpireCities : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class EmpireUnitOrder : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

namespace empire_evaluations {
  EmpireColonize& get_colonize();
  EmpireProduce& get_produce();
  EmpireUnits& get_units();
  EmpireCities& get_cities();
  EmpireUnitOrder& get_unit_order();
}
