#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>

// NOTE: Add new resource types to resource_type_names in resources.cpp
enum class RESOURCE_TYPE {
  UNKNOWN,
  GOLD,
  HAPPINESS,
  SUGAR,
  STONE,

  FIRST = GOLD,
  LAST = STONE,
};

const char* get_resource_name(RESOURCE_TYPE resource);

// These resources will represent civilization level resources. 
// Examples: Amount of gold in a civilization, amount of happiness.
// Or a strategic resource:
// Amount of sugar or stone, for example.
struct Resource {
  Resource() : m_type(RESOURCE_TYPE::UNKNOWN), m_quantity(0) {};
  Resource(RESOURCE_TYPE type) : m_type(type), m_quantity(0) {};
  Resource(RESOURCE_TYPE type, uint32_t quantity) : m_type(type), m_quantity(quantity) {};

  Resource& operator+=(const Resource& rhs);

  // Idicates the type of resource this is.
  RESOURCE_TYPE m_type;
  // Some resources may wnat to go negative. Money for instance, depending upon design.
  int32_t m_quantity;
};

class Resources {
public:
  Resources();
 
  // Provide a negative quantity to deduct resources. 
  void add(RESOURCE_TYPE type, int32_t quantity);
  // Adds the quantity in resource to the appropriate entry.
  void add(Resource resource);

  void for_each_resource(std::function<void(RESOURCE_TYPE type, const Resource& resource)> operation) const;

  // The key is the enum RESOURCE_TYPE
  typedef std::unordered_map<uint32_t, Resource> ResourceMap;
  ResourceMap m_resource_map;
};
