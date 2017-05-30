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

ResourceUMap::ResourceUMap() {
  auto check = ([this](fbs::RESOURCE_TYPE type){
    this->m_resource_map[util::enum_to_uint(type)] = Resource(type);
  });
  for (auto rt : fbs::EnumValuesRESOURCE_TYPE()) {
    check(rt);
  }
}

void ResourceUMap::add(fbs::RESOURCE_TYPE type, int32_t quantity) {
  m_resource_map[util::enum_to_uint(type)].m_quantity += quantity;
}

void ResourceUMap::add(Resource resource) {
  m_resource_map[util::enum_to_uint(resource.m_type)] += resource;
}

void ResourceUMap::for_each_resource(std::function<void(fbs::RESOURCE_TYPE type, const Resource& resource)> operation) const {
  for (auto resource : m_resource_map) {
    operation(util::uint_to_enum<fbs::RESOURCE_TYPE>(resource.first), resource.second);
  }
}
