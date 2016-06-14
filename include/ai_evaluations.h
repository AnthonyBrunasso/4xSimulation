#pragma once

#include "dtree.h"

class NeedsColonize : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class NeedsProduce : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class HasUnits : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};

class DiscoveredCities : public Evaluation {
  virtual float operator()(uint32_t player_id, float threshold) override;
};
