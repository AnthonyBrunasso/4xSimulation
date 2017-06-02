#include "improvement.h"

#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Vector3.hpp"
#include "step_generated.h"
#include "unique_id.h"
#include "util.h"

namespace {
  typedef std::unordered_map<uint32_t, Improvement*> ImprovementMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  typedef std::vector<std::function<bool(fbs::RESOURCE_TYPE, fbs::IMPROVEMENT_TYPE, const sf::Vector3i&)> > Requirements;
  typedef std::unordered_map<uint32_t, Requirements> RequirementMap;
  typedef std::unordered_map<uint32_t, uint32_t> ResourceImprovementMap;
  typedef std::vector<std::uint32_t> ValidResourceVector;
  typedef std::unordered_map<uint32_t, ValidResourceVector> ImprovementResourcesMap;
  ImprovementResourcesMap s_impvResources;
  ImprovementMap s_improvements;
  SubMap s_destroy_subs;
  SubMap s_create_subs;
  RequirementMap s_creation_requirements;
  ResourceImprovementMap s_resource_improvements;
}

void improvement::initialize() {
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::LUXURY_GOLD)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::LUXURY_SUGAR)] = any_enum(fbs::IMPROVEMENT_TYPE::PLANTATION);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STRATEGIC_IRON)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STRATEGIC_COAL)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::CATTLE)] = any_enum(fbs::IMPROVEMENT_TYPE::PASTURE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::DEER)] = any_enum(fbs::IMPROVEMENT_TYPE::CAMP);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::FISH)] = any_enum(fbs::IMPROVEMENT_TYPE::FISH_BOATS);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STONE)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::SHEEP)] = any_enum(fbs::IMPROVEMENT_TYPE::PASTURE);

  for (auto& res : s_resource_improvements) {
    s_impvResources[res.second].push_back(res.first);
  }
}

Improvement::Improvement(uint32_t unique_id, Resource res, fbs::IMPROVEMENT_TYPE type) 
  : m_id(unique_id)
  , m_resource(res)
  , m_type(type)
  , m_owner_id(unique_id::INVALID_ID)
{
}

void improvement::add_requirement(fbs::IMPROVEMENT_TYPE type, 
    std::function<bool(fbs::RESOURCE_TYPE, fbs::IMPROVEMENT_TYPE, const sf::Vector3i&)> requirement) {
  s_creation_requirements[any_enum(type)].push_back(requirement);
}

bool improvement::satisfies_requirements(fbs::RESOURCE_TYPE rtype
    , fbs::IMPROVEMENT_TYPE itype
    , const sf::Vector3i& location) {
  Requirements& requirements = s_creation_requirements[any_enum(itype)]; 
  // Verify all requirements are satisfied for this improvement.
  for (auto requirement : requirements) {
    if (!requirement(rtype, itype, location)) {
      std::cout << "Could not satisfy improvement create requirements." << std::endl;
      return false;
    }
  }
  return true;
}

uint32_t improvement::create(Resource res
    , fbs::IMPROVEMENT_TYPE type
    , const sf::Vector3i& location
    , uint32_t owner) {
  if (!satisfies_requirements(res.m_type, type, location)) {
    return unique_id::INVALID_ID;
  }

  uint32_t id = unique_id::generate();
  Improvement* improvement = new Improvement(id, res, type);
  improvement->m_location = location;
  improvement->m_owner_id = owner;

  s_improvements[id] = improvement;
  std::cout << "Created improvement id " << id << ", improvement type: " << fbs::EnumNameIMPROVEMENT_TYPE(type) << std::endl;

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

ValidResourceVector improvement::resource_requirements(fbs::IMPROVEMENT_TYPE type) {
  ValidResourceVector none;
  uint32_t impv = any_enum(type);
  ImprovementResourcesMap::const_iterator itFind = s_impvResources.find(impv);
  if (itFind == s_impvResources.end()) {
    return none;
  }

  return itFind->second;
}

fbs::IMPROVEMENT_TYPE improvement::resource_improvement(fbs::RESOURCE_TYPE resource) {
  uint32_t rid = any_enum(resource);
  return any_enum(s_resource_improvements[rid]);
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

void improvement::reset() {
  for (auto& i : s_improvements) {
    delete i.second;
  }
  s_improvements.clear();
  s_impvResources.clear();
  s_destroy_subs.clear();
  s_create_subs.clear();
  s_creation_requirements.clear();
  s_resource_improvements.clear();
}

