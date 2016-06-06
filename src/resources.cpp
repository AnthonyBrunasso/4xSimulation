#include "resources.h"

#include <iostream>

#include "util.h"

namespace {
#define MAKE_STRING(name) # name

  const char* resource_type_names[] = {
    MAKE_STRING(RESOURCE_TYPE::GOLD),
    MAKE_STRING(RESOURCE_TYPE::HAPPINESS),
  };
}

const char* print_resource_name(RESOURCE_TYPE resource) {
  if (resource < RESOURCE_TYPE::FIRST || resource > RESOURCE_TYPE::LAST) {
    return "Invalid resource type";
  }
  return resource_type_names[util::enum_to_uint(resource)];
}

Resources::Resources() {
  int i = util::enum_to_uint(RESOURCE_TYPE::FIRST), e = util::enum_to_uint(RESOURCE_TYPE::LAST);
  do { 
    m_resource_map[i] = Resource();
  } while(i++ != e);
}

void Resources::add(RESOURCE_TYPE type, int32_t quantity) {
  m_resource_map[util::enum_to_uint(type)].m_quantity += quantity;
}

void Resources::for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const {
  for (auto resource : m_resource_map) {
    operation(util::uint_to_enum<RESOURCE_TYPE>(resource.first), resource.second);
  }
}
