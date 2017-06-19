#include "resources.h"

#include <utility>

#include "step_generated.h"
#include "util.h"

Resource::Resource()
  : m_type(fbs::RESOURCE_TYPE::UNKNOWN)
  , m_quantity(0) {};

Resource::Resource(fbs::RESOURCE_TYPE type)
  : m_type(type),
  m_quantity(0) {};

Resource::Resource(fbs::RESOURCE_TYPE type, uint32_t quantity)
  : m_type(type),
  m_quantity(quantity) {};

bool Resource::operator==(const Resource& rhs) const {
  return m_type == rhs.m_type;
}

Resource& Resource::operator+=(const Resource& rhs) {
  // Addition with non matching types do nothing.
  if (rhs.m_type != m_type) {
    return *this;
  }
  m_quantity += rhs.m_quantity;
  return *this;
}

