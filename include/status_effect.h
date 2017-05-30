#pragma once

#include "step_generated.h"
#include "Vector3.hpp"

#include <cstdint>
#include <vector>
#include <functional>

class Tile;

class StatusEffect {
public:
  StatusEffect(uint32_t id, fbs::STATUS_TYPE type, const sf::Vector3i& location) :
    m_id(id)
    , m_type(type)
    , m_location(location)
    , m_turns(0)
    , m_current_turn(m_turns) {};

  virtual ~StatusEffect() {};

  // Begin turn will run on the first turn the status effect is in play.
  // The first turn of a status effect starts the turn *after* it is played.
  virtual void begin_turn() {};

  // End turn is defined by m_current_turn equalling 0.
  virtual void end_turn() {};

  // Per turn will run every begin_turn step after the status effect comes into play.
  virtual void per_turn() {};

  // Status effects dictate who they spread to.
  virtual void spread(Tile& tile) = 0;

  uint32_t m_id;
  fbs::STATUS_TYPE m_type;
  // Origination location
  sf::Vector3i m_location;
  uint32_t m_range;
  uint32_t m_turns;
  uint32_t m_current_turn;

  // An end turn can be injected into a status effect on creation.
  std::function<void()> m_begin_turn_injection;
  std::function<void()> m_per_turn_injection;
  std::function<void()> m_end_turn_injection;

  // Targets
  std::vector<sf::Vector3i> m_tiles;
  std::vector<uint32_t> m_units;
  std::vector<uint32_t> m_cities;
};

namespace status_effect {
  // The next status effect created will inject these std functions into 
  // its begin turn, end turn and per turn members.
  void inject(std::function<void()> begin, std::function<void()> end, std::function<void()> per);
  void inject_begin(std::function<void()> begin);
  void inject_end(std::function<void()> end);
  void inject_per(std::function<void()> per);

  uint32_t create(fbs::STATUS_TYPE type, const sf::Vector3i& location);  
  void sub_create(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  void destroy(uint32_t id);
  void sub_destroy(std::function<void(const sf::Vector3i&, uint32_t)> sub);

  StatusEffect* get_effect(uint32_t id);
  void for_each_effect(std::function<void(const StatusEffect& effect)> operation);

  void process(); 

  void reset();
}
