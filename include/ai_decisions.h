#pragma once

#include "dtree.h"

#include "game_types.h"

class Settle : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

class Construct : public Decision {
  virtual void operator()(uint32_t player_id) override;

  CONSTRUCTION_TYPE m_production_type;
};

class Explore : public Decision {
  virtual void operator()(uint32_t player_id) override;
};

