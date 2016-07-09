// Named it custom_math.h so there are no conflicts with C's math.h

#pragma once

namespace cmath {
  template <typename LERP_TYPE>
  LERP_TYPE lerp(LERP_TYPE a, LERP_TYPE b, float t) {
    return (1.0f - t) * a + t * b;  
  }

  template <typename VECTOR>
  float dot(const VECTOR& lhs, const VECTOR& rhs) {
    return static_cast<float>(lhs.x) * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }
}
