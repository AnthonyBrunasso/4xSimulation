#include "improvement.h"

#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Vector3.hpp"
#include "step_generated.h"
#include "unique_id.h"
#include "util.h"
#include "world_map.h"

namespace improvement {
  typedef std::unordered_map<uint32_t, Improvement*> ImprovementMap;
  typedef std::unordered_map<uint32_t, uint32_t> ResourceImprovementMap;
  typedef std::vector<std::uint32_t> ValidResourceVector;
  ImprovementMap s_improvements;
  ResourceImprovementMap s_resource_improvements;

  typedef std::function<bool(const sf::Vector3i&, uint32_t)> SubscriberFunc;
  constexpr size_t SUBSCRIBER_LIMIT = 10;
  SubscriberFunc s_destroy_subs[SUBSCRIBER_LIMIT];
  SubscriberFunc s_create_subs[SUBSCRIBER_LIMIT];

  bool valid_resource(fbs::RESOURCE_TYPE selected_type, fbs::IMPROVEMENT_TYPE type);
  bool is_resource_available(fbs::RESOURCE_TYPE rt, const sf::Vector3i& location);
}

void improvement::initialize() {
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::LUXURY_GOLD)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::LUXURY_SUGAR)] = any_enum(fbs::IMPROVEMENT_TYPE::PLANTATION);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STRATEGIC_IRON)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STRATEGIC_COAL)] = any_enum(fbs::IMPROVEMENT_TYPE::MINE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::CATTLE)] = any_enum(fbs::IMPROVEMENT_TYPE::PASTURE);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::DEER)] = any_enum(fbs::IMPROVEMENT_TYPE::CAMP);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::FISH)] = any_enum(fbs::IMPROVEMENT_TYPE::FISH_BOATS);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::STONE)] = any_enum(fbs::IMPROVEMENT_TYPE::QUARRY);
  s_resource_improvements[any_enum(fbs::RESOURCE_TYPE::SHEEP)] = any_enum(fbs::IMPROVEMENT_TYPE::PASTURE);
}

Improvement::Improvement(uint32_t unique_id, Resource res, fbs::IMPROVEMENT_TYPE type) 
  : m_id(unique_id)
  , m_resource(res)
  , m_type(type)
  , m_owner_id(unique_id::INVALID_ID)
{
}

bool improvement::is_resource_available(fbs::RESOURCE_TYPE rt, const sf::Vector3i& location) {
	Tile* tile = world_map::get_tile(location);
	if (!tile) {
		std::cout << "Invalid tile" << std::endl;
		return false;
	}

	// Check if this tile already contains a resource improvement.
	for (auto id : tile->m_improvement_ids) {
		Improvement* improvement = improvement::get_improvement(id);
		if (!improvement) continue;
		if (improvement->m_resource.m_type == rt) {
			std::cout << "Resource improvement already exists on this tile" << std::endl;
			return false;
		}
	}

	return true;
}

bool improvement::valid_resource(fbs::RESOURCE_TYPE selected_type, fbs::IMPROVEMENT_TYPE type) {
	return type == improvement::resource_improvement(selected_type);
}

bool improvement::satisfies_requirements(fbs::RESOURCE_TYPE rtype
    , fbs::IMPROVEMENT_TYPE itype
    , const sf::Vector3i& location) {
  return valid_resource(rtype, itype) && is_resource_available(rtype, location);
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
    if (sub) {
      sub(location, id);
    }
  }

  return id;
}

void improvement::sub_create(std::function<bool(const sf::Vector3i&, uint32_t)> sub) {
  for (auto& s : s_create_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
}

void improvement::destroy(uint32_t id) {
  Improvement* improvement = get_improvement(id);
  if (!improvement) {
    return;
  }

  for (auto sub : s_destroy_subs) {
    if (sub) {
      sub(improvement->m_location, id);
    }
  }

  s_improvements.erase(id);
  delete improvement;
}

void improvement::sub_destroy(std::function<bool(const sf::Vector3i&, uint32_t)> sub) {
  for (auto& s : s_destroy_subs) {
    if (!s) {
      s = sub;
      return;
    }
  }

  std::cout << "Error: Subscriber Limit" << std::endl;
}

improvement::ValidResourceVector improvement::resource_requirements(fbs::IMPROVEMENT_TYPE type) {
  ValidResourceVector valid_resources;
  uint32_t imp_id = any_enum(type);
  for (auto it : s_resource_improvements) {
    if (it.second == imp_id) {
      valid_resources.push_back(it.first);
    }
  }

  return valid_resources;
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
  for (auto& s : s_destroy_subs) {
    s = SubscriberFunc();
  }
  for (auto& s : s_create_subs) {
    s = SubscriberFunc();
  }
  s_resource_improvements.clear();
}

