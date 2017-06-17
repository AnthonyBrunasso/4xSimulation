#pragma once

#include <Vector3.hpp>
#include <cstdint>
#include <functional>
#include <vector>

#include "enum_generated.h"
#include "resources.h"

struct Improvement {
  Improvement(uint32_t unique_id, Resource res, fbs::IMPROVEMENT_TYPE type);

  uint32_t m_id;
  Resource m_resource;
  fbs::IMPROVEMENT_TYPE m_type;
  // This is the player that owns this resource
  uint32_t m_owner_id;
  sf::Vector3i m_location;
};

namespace improvement {
  typedef std::vector<std::uint32_t> ValidResourceVector;

  void initialize();

  // Improvements will often have requirements. For example, improving a resource
  // requires a resource exist on that tile. Building a road requires that the player
  // the necessary research to build it.
  bool satisfies_requirements(fbs::RESOURCE_TYPE rtype
      , fbs::IMPROVEMENT_TYPE itype
      , const sf::Vector3i& location);

  uint32_t create(Resource res
      , fbs::IMPROVEMENT_TYPE type
      , const sf::Vector3i& location
      , uint32_t owner);

  void sub_create(std::function<bool(const sf::Vector3i&, uint32_t)> sub);

  void destroy(uint32_t id);
  void sub_destroy(std::function<bool(const sf::Vector3i&, uint32_t)> sub);

  ValidResourceVector resource_requirements(fbs::IMPROVEMENT_TYPE type);
  fbs::IMPROVEMENT_TYPE resource_improvement(fbs::RESOURCE_TYPE resource);

  void for_each_improvement(
      std::function<void(const Improvement& improvement)> operation);
  Improvement* get_improvement(uint32_t id);

  void reset();
}
