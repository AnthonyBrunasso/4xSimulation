#pragma once

#include <Vector3.hpp>
#include <stdint.h>

#include "enum_generated.h"

namespace flatbuffers {
  class FlatBufferBuilder;
  template <typename T> class Offset;
}

namespace fbs {
  enum class StepUnion : uint8_t;
}

namespace ai_shared {
  flatbuffers::FlatBufferBuilder& GetFBB();
  void simulate_step(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step);

  void approach(uint32_t unit_id, const sf::Vector3i& location); 
  bool attack_unit(uint32_t unit_id, uint32_t target_id);
  bool attack_city(uint32_t unit_id, uint32_t city_id);
  bool approach_unit(uint32_t unit_id, uint32_t target_id);
  bool approach_city(uint32_t unit_id, uint32_t target_id);
  bool pillage_improvement(uint32_t unit_id, uint32_t target_id);
  bool wander(uint32_t unit_id);
  bool approach_improvement(uint32_t unit_id, uint32_t target_id);
  sf::Vector3i get_random_coord();
}
