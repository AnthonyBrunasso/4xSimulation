#pragma once

#include <stdint.h>

#include "dtree.h"

class EmpireColonize : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class EmpireProduce : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

// Checks if any cities have been discover.
class DiscoveredCities : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class EmpireUnitOrder : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class IdleMilitaryUnits : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class DefenderVsIdle : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class Fortified : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class ThreatenedVsAvailable : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class ApproachVsAttack : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class FindCityVsGotoCity : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class PillageVsSiege : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class ApproachVsSiege : public Evaluation {
public:
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class ApproachVsPillage : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

namespace empire_evaluations {
  EmpireColonize& get_colonize();
  EmpireProduce& get_produce();
  DiscoveredCities& get_discovered_cities();
  EmpireUnitOrder& get_unit_order();

  // has functions use "no" as left "yes" as right
  IdleMilitaryUnits& has_idle_military();
  DefenderVsIdle& defender_vs_idle();
  Fortified& has_fortified();
  ThreatenedVsAvailable& threatened_vs_available();
  ApproachVsAttack& approach_vs_attack();
  FindCityVsGotoCity& find_city_vs_goto_city();
  PillageVsSiege& pillage_vs_siege();
  ApproachVsSiege& approach_vs_siege();
  ApproachVsPillage& approach_vs_pillage();
}
