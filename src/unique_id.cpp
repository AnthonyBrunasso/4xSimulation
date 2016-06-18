#include "unique_id.h"

namespace {
  // Something simple, start it at 1, INVALID_ID = 0
  uint32_t s_unique_id = unique_id::INVALID_ID + 1;
}

uint32_t unique_id::generate() {
  return s_unique_id++;
}

uint32_t unique_id::get_next() {
  return s_unique_id;
}
