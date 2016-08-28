#pragma once

#include "dtree.h"

class UnitFindNear : public Evaluation {
  virtual float operator()(uint32_t unit_id, float threshold) override;
};

namespace unit_evaluations {
  UnitFindNear& get_find_near();
}