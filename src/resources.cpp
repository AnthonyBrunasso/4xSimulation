#include "resources.h"

#include <iostream>

namespace {
#define MAKE_STRING(name) # name

  const char* resource_type_names[] = {
    MAKE_STRING(RESOURCE_TYPE::GOLD),
    MAKE_STRING(RESOURCE_TYPE::HAPPINESS),
  };

  uint32_t to_uint(RESOURCE_TYPE type) {
    return static_cast<uint32_t>(type);
  }

  RESOURCE_TYPE to_resource(uint32_t uint) {
    return static_cast<RESOURCE_TYPE>(uint);
  }
}

const char* print_resource_name(RESOURCE_TYPE resource) {
  if (resource < RESOURCE_TYPE::FIRST || resource > RESOURCE_TYPE::LAST) {
    return "Invalid resource type";
  }
  return resource_type_names[to_uint(resource)];
}

Resources::Resources() {
  int i = to_uint(RESOURCE_TYPE::FIRST), e = to_uint(RESOURCE_TYPE::LAST);
  do { 
    m_resource_map[i] = Resource();
  } while(i++ != e);
}

void Resources::add(RESOURCE_TYPE type, int32_t quantity) {
  m_resource_map[to_uint(type)].m_quantity += quantity;
}

void Resources::for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const {
  for (auto resource : m_resource_map) {
    operation(to_resource(resource.first), resource.second);
  }
}
