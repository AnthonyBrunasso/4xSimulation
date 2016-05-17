#pragma once

#include <cstdint>

namespace unique_id {
  // 0 is an invalid id, makes invalid easily recognizable when debugging
  static const uint32_t INVALID_ID = 0;
  // Returns a unique id on each call
  uint32_t generate();
}