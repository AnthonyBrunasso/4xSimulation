#include "resources.h"

#include <iostream>

#include "util.h"

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
  for_each_resource_type([this](RESOURCE_TYPE type){
    this->m_resource_map[util::enum_to_uint(type)] = Resource(type);
  });
}

void ResourceUMap::add(RESOURCE_TYPE type, int32_t quantity) {
  m_resource_map[util::enum_to_uint(type)].m_quantity += quantity;
}

void ResourceUMap::add(Resource resource) {
  m_resource_map[util::enum_to_uint(resource.m_type)] += resource;
}

void ResourceUMap::for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const {
  for (auto resource : m_resource_map) {
    operation(util::uint_to_enum<RESOURCE_TYPE>(resource.first), resource.second);
  }
}
