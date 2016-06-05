#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>

// Temporary resource types.
// Keep resources continuous, they are iterated over in Resources constructor.
// NOTE: Add new resource types to resource_type_names in resources.cpp
enum class RESOURCE_TYPE {
  GOLD,
  HAPPINESS,

  FIRST = GOLD,
  LAST = HAPPINESS,
};

const char* print_resource_name(RESOURCE_TYPE resource);

struct Resource {
  Resource() : m_quantity(0) {};

  // Some resources may wnat to go negative. Money for instance, depending upon design.
  int32_t m_quantity;
};

class Resources {
public:
  Resources();
 
  // Provide a negative quantity to deduct resources. 
  void add(RESOURCE_TYPE type, int32_t quantity);
  void for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const;

  // The key is the enum RESOURCE_TYPE
  typedef std::unordered_map<uint32_t, Resource> ResourceMap;
  ResourceMap m_resource_map;
};
