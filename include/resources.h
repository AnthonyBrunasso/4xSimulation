#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>



namespace fbs {
  enum class RESOURCE_TYPE : uint32_t;
}

// These resources will represent civilization level resources. 
// Examples: Amount of gold in a civilization, amount of happiness.
// Or a strategic resource:
// Amount of sugar or stone, for example.
struct Resource {
  explicit Resource();
  explicit Resource(fbs::RESOURCE_TYPE type);
  explicit Resource(fbs::RESOURCE_TYPE type, uint32_t quantity);

  bool operator==(const Resource& rhs) const;
  Resource& operator+=(const Resource& rhs);

  // Idicates the type of resource this is.
  fbs::RESOURCE_TYPE m_type;
  // Some resources may wnat to go negative. Money for instance, depending upon design.
  int32_t m_quantity;
};

class ResourceUMap {
public:
  ResourceUMap();
 
  // Provide a negative quantity to deduct resources. 
  void add(fbs::RESOURCE_TYPE type, int32_t quantity);
  // Adds the quantity in resource to the appropriate entry.
  void add(Resource resource);

  void for_each_resource(std::function<void(fbs::RESOURCE_TYPE type, const Resource& resource)> operation) const;

  // The key is the enum fbs::RESOURCE_TYPE
  typedef std::unordered_map<uint32_t, Resource> ResourceMap;
  ResourceMap m_resource_map;
};
