#pragma once

#include <cstdint>

namespace unique_id {
  // 0 is an invalid id, makes invalid easily recognizable when debugging
  static const uint32_t INVALID_ID = 0;
  static const uint32_t INVALID_PLAYER = 0xffffffff;
  // Returns a unique id on each call
  uint32_t generate();
  // Gets the next valid id that will be used when generate is called.
  uint32_t get_next();

  void reset();
}
