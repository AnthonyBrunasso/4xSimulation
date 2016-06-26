#pragma once

#include <Vector3.hpp>
#include <cstdint>
#include <functional>
#include <vector>

enum class RESOURCE_TYPE;
enum class IMPROVEMENT_TYPE;

struct Improvement {
  Improvement(uint32_t unique_id, RESOURCE_TYPE res, IMPROVEMENT_TYPE type);

  uint32_t m_unique_id;
  RESOURCE_TYPE m_resource;
  IMPROVEMENT_TYPE m_type;
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
  void add_requirement(IMPROVEMENT_TYPE type, std::function<bool(RESOURCE_TYPE, IMPROVEMENT_TYPE, const sf::Vector3i&)> requirement);
  uint32_t create(RESOURCE_TYPE res, IMPROVEMENT_TYPE type, const sf::Vector3i& location, uint32_t owner);
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  void destroy(uint32_t id);
  void sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  ValidResourceVector resource_requirements(IMPROVEMENT_TYPE type);

  void for_each_improvement(
      std::function<void(const Improvement& improvement)> operation);
  Improvement* get_improvement(uint32_t id);
}
