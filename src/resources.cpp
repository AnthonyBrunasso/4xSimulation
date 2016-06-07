#include "resources.h"

#include <iostream>

#include "util.h"

namespace {
  const char* resource_type_names[] = {
    "Unknown",
    "Gold",
    "Happiness",
    "Sugar",
    "Stone",
  };
}

Resource& Resource::operator+=(const Resource& rhs) {
  // Addition with non matching types do nothing.
  if (rhs.m_type != m_type) {
    return *this;
  }
  m_quantity += rhs.m_quantity;
  return *this;
}

const char* get_resource_name(RESOURCE_TYPE resource) {
  std::cout << "TRYING: " << util::enum_to_uint(resource) << std::endl;
  if (resource < RESOURCE_TYPE::FIRST || resource > RESOURCE_TYPE::LAST) {
    return "Invalid resource type";
  }
  return resource_type_names[util::enum_to_uint(resource)];
}

Resources::Resources() {
  int i = util::enum_to_uint(RESOURCE_TYPE::FIRST), e = util::enum_to_uint(RESOURCE_TYPE::LAST);
  do { 
    m_resource_map[i] = Resource(util::uint_to_enum<RESOURCE_TYPE>(i));
  } while(i++ != e);
}

void Resources::add(RESOURCE_TYPE type, int32_t quantity) {
  m_resource_map[util::enum_to_uint(type)].m_quantity += quantity;
}

void Resources::add(Resource resource) {
  m_resource_map[util::enum_to_uint(resource.m_type)] += resource;
}

void Resources::for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const {
  for (auto resource : m_resource_map) {
    operation(util::uint_to_enum<RESOURCE_TYPE>(resource.first), resource.second);
  }
}
