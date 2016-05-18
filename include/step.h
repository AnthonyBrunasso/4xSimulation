#pragma once

#include <cstdint>
#include <string>

#include "Vector3.hpp"

enum class COMMAND {
  QUIT,
  BEGIN_TURN,
  END_TURN,
  ATTACK,
  COLONIZE,
  CONSTRUCT,
  DISCOVER,
  IMPROVE,
  KILL,
  MOVE,
  PURCHASE,
  SELL,
  SPAWN,
  ADD_PLAYER,
};

struct Step {
  Step(COMMAND command) : m_command(command) {};
  virtual ~Step() {};

  COMMAND m_command;
};

struct SpawnStep : public Step {
  SpawnStep(COMMAND command) : Step(command)
    , m_entity_type(0)
    , m_location()
    , m_player(0) {};

  uint32_t m_entity_type;
  sf::Vector3i m_location;
  uint32_t m_player;
};

struct ColonizeStep : public Step {
  ColonizeStep(COMMAND command) : Step(command)
    , m_unit_id(0)
    , m_location()
    , m_player(0) {};
    
  uint32_t m_unit_id;
  sf::Vector3i m_location;
  uint32_t m_player;
};

struct MoveStep : public Step {
  MoveStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
  sf::Vector3i m_destination;
};

struct AddPlayerStep : public Step {
  AddPlayerStep(COMMAND command) : Step(command) {};
  std::string m_name;
};
