#pragma once

#include "dtree.h"

#include "game_types.h"

class Settle : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class Construct : public Decision {
public:
  Construct(CONSTRUCTION_TYPE type) : m_production_type(type) {}; 

  virtual void operator()(uint32_t player_id) override;

  CONSTRUCTION_TYPE m_production_type;
};

class Explore : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

class UnitDecision : public Decision {
public:
  virtual void operator()(uint32_t player_id) override;
};

