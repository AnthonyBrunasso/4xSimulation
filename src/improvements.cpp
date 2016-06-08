#include "improvements.h"

#include "unique_id.h"
#include "util.h"

#include <unordered_map>
#include <vector>
#include <iostream>

namespace {
  const char* s_improvement_type_name[] = {
    "UNKOWN",
    "RESOURCE",
  };

  typedef std::unordered_map<uint32_t, Improvement*> ImprovementMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  typedef std::vector<std::function<bool(const sf::Vector3i&)> > Requirements;
  typedef std::unordered_map<uint32_t, Requirements> RequirementMap;
  ImprovementMap s_improvements;
  SubMap s_destroy_subs;
  SubMap s_create_subs;
  RequirementMap s_creation_requirements;
}

Improvement::Improvement(uint32_t unique_id, IMPROVEMENT_TYPE type) :
  m_unique_id(unique_id)
  , m_type(type)
  , m_owner_id(unique_id::INVALID_ID)
{
}

const char* get_improvement_name(IMPROVEMENT_TYPE improvement) {
  if (improvement > IMPROVEMENT_TYPE::LAST) {
    return "Unkown resource.";
  }
  return s_improvement_type_name[util::enum_to_uint(improvement)];
}

void improvement::add_requirement(IMPROVEMENT_TYPE type, 
    std::function<bool(const sf::Vector3i&)> requirement) {
  s_creation_requirements[util::enum_to_uint(type)].push_back(requirement);
}

uint32_t improvement::create(IMPROVEMENT_TYPE type, const sf::Vector3i& location) {
  Requirements& requirements = s_creation_requirements[util::enum_to_uint(type)]; 
  // Verify all requirements are satisfied for this improvement.
  for (auto requirement : requirements) {
    if (!requirement(location)) {
      std::cout << "Could not satisfy improvement requirements." << std::endl;
      return false;
    }
  }

  uint32_t id = unique_id::generate();
  Improvement* improvement = new Improvement(id, type);
  improvement->m_location = location;

  s_improvements[id] = improvement;
  std::cout << "Created improvement id " << id << ", improvement type: " << get_improvement_name(type) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void improvement::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void improvement::destroy(uint32_t id) {
  Improvement* improvement = get_improvement(id);
  if (!improvement) {
    return;
  }

  for (auto sub : s_destroy_subs) {
    sub(improvement->m_location, id);
  }

  s_improvements.erase(id);
  delete improvement;
}

void improvement::sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_destroy_subs.push_back(sub);
}

Improvement* improvement::get_improvement(uint32_t id) {
  if (s_improvements.find(id) == s_improvements.end()) {
    return nullptr;
  }
  return s_improvements[id];
}

void improvement::for_each_improvement(
    std::function<void(const Improvement& improvement)> operation) {
  for (auto improvement : s_improvements) {
    operation(*improvement.second);
  }
}
