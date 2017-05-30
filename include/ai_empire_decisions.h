#pragma once

#include <stdint.h>

#include "dtree.h"
#include "game_types.h"

namespace fbs {
  enum class CONSTRUCTION_TYPE : uint32_t;
}

class EmpireSettle : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class EmpireConstruct : public Decision {
public:
  EmpireConstruct(fbs::CONSTRUCTION_TYPE type) : m_production_type(type) {}; 

  virtual void operator()(uint32_t player_id) override;

  fbs::CONSTRUCTION_TYPE m_production_type;
};

class EmpireExplore : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class EmpireUnitDecisions : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class EmpireEndTurn : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpireFortifyUnit : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpireApproach : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpireAttack : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpireSiege : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpirePillage : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class EmpireWander : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

namespace empire_decisions {
  EmpireSettle& get_settle();
  EmpireConstruct& get_construct();
  EmpireExplore& get_explore();
  EmpireUnitDecisions& get_unit_decisions();

  EmpireEndTurn& decide_endturn();
  EmpireFortifyUnit& decide_fortify();
  EmpireApproach& decide_approach();
  EmpireAttack& decide_attack();
  EmpireSiege& decide_siege();
  EmpirePillage& decide_pillage();
  EmpireWander& decide_wander();
}
