#include "status_effect.h"

#include <iostream>
#include <unordered_map>

namespace {
  typedef std::unordered_map<uint32_t, StatusEffect*> StatusMap;
  typedef std::vector<std::function<void(const sf::Vector3i&, uint32_t)> > SubMap;
  StatusMap s_status;
  SubMap s_create_subs;
  SubMap s_destroy_subs;

  // Status effects don't use the global unique_id ids.
  uint32_t s_unique_ids = 1;
}

uint32_t status_effect::create(STATUS_TYPE type, const sf::Vector3i& location) {
  uint32_t id = s_unique_ids++;
  StatusEffect* e = new StatusEffect(id, type, location);
  s_status[id] = e;

  std::cout << "Created status effect id " << id << " type: " << get_status_name(type) << std::endl;

  for (auto sub : s_create_subs) {
    sub(location, id);
  }

  return id;
}

void status_effect::sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_create_subs.push_back(sub);
}

void status_effect::destroy(uint32_t id) {
  StatusEffect* e = get_effect(id);
  if (!e) return;

  for (auto sub : s_destroy_subs) {
    sub(e->m_location, id);
  }

  s_status.erase(id);
  delete e;
}
void status_effect::sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub) {
  s_destroy_subs.push_back(sub);
}

StatusEffect* status_effect::get_effect(uint32_t id) {
  if (s_status.find(id) == s_status.end()) return nullptr;
  return s_status[id];
}

void status_effect::for_each_effect(std::function<void(const StatusEffect& effect)> operation) {
  for (auto status : s_status) {
    operation(*status.second);
  }
}

