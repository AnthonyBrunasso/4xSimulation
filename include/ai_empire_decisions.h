#pragma once

#include "dtree.h"

#include "game_types.h"

class EmpireSettle : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class EmpireConstruct : public Decision {
public:
  EmpireConstruct(CONSTRUCTION_TYPE type) : m_production_type(type) {}; 

  virtual void operator()(uint32_t player_id) override;

  CONSTRUCTION_TYPE m_production_type;
};

class EmpireExplore : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class EmpireUnitDecisions : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

namespace empire_decisions {
  EmpireSettle& get_settle();
  EmpireConstruct& get_construct();
  EmpireExplore& get_explore();
  EmpireUnitDecisions& get_unit_decisions();
}