#include "improvement.h"

#include "unique_id.h"
#include "util.h"

#include <unordered_map>
#include <vector>
#include <iostream>

namespace {
  typedef std::unordered_map<uint32_t, Improvement*> ImprovementMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  typedef std::vector<std::function<bool(RESOURCE_TYPE, IMPROVEMENT_TYPE, const sf::Vector3i&)> > Requirements;
  typedef std::unordered_map<uint32_t, Requirements> RequirementMap;
  typedef std::vector<std::uint32_t> ValidResourceVector;
  typedef std::unordered_map<uint32_t, ValidResourceVector> ImprovementResourcesMap;
  ImprovementResourcesMap s_impvResources;
  ImprovementMap s_improvements;
  SubMap s_destroy_subs;
  SubMap s_create_subs;
  RequirementMap s_creation_requirements;
}

void improvement::initialize() {
  ValidResourceVector mining = { 
    util::enum_to_uint(RESOURCE_TYPE::LUXURY_GOLD),
    util::enum_to_uint(RESOURCE_TYPE::STRATEGIC_IRON),
    util::enum_to_uint(RESOURCE_TYPE::STRATEGIC_COAL),
  };
  ValidResourceVector pasture = {
    util::enum_to_uint(RESOURCE_TYPE::CATTLE),
  };
  ValidResourceVector camp = {
    util::enum_to_uint(RESOURCE_TYPE::DEER),
  };
  ValidResourceVector quarry = {
    util::enum_to_uint(RESOURCE_TYPE::STONE),
  };
  ValidResourceVector fish_boats = {
    util::enum_to_uint(RESOURCE_TYPE::FISH),
  };
  ValidResourceVector plantation = {
    util::enum_to_uint(RESOURCE_TYPE::LUXURY_SUGAR),
  };
  
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::MINE)] = mining;
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::PASTURE)] = pasture;
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::CAMP)] = camp;
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::PLANTATION)] = plantation;
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::QUARRY)] = quarry;
  s_impvResources[util::enum_to_uint(IMPROVEMENT_TYPE::FISH_BOATS)] = fish_boats;
}

Improvement::Improvement(uint32_t unique_id, Resource res, IMPROVEMENT_TYPE type) 
  : m_id(unique_id)
  , m_resource(res)
  , m_type(type)
  , m_owner_id(unique_id::INVALID_ID)
{
}

void improvement::add_requirement(IMPROVEMENT_TYPE type, 
    std::function<bool(RESOURCE_TYPE, IMPROVEMENT_TYPE, const sf::Vector3i&)> requirement) {
  s_creation_requirements[util::enum_to_uint(type)].push_back(requirement);
}

bool improvement::satisfies_requirements(RESOURCE_TYPE rtype
    , IMPROVEMENT_TYPE itype
    , const sf::Vector3i& location) {
  Requirements& requirements = s_creation_requirements[util::enum_to_uint(itype)]; 
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
    , IMPROVEMENT_TYPE type
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

ValidResourceVector improvement::resource_requirements(IMPROVEMENT_TYPE type) {
  ValidResourceVector none;
  uint32_t impv = util::enum_to_uint(type);
  ImprovementResourcesMap::const_iterator itFind = s_impvResources.find(impv);
  if (itFind == s_impvResources.end()) {
    return none;
  }

  return itFind->second;
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

