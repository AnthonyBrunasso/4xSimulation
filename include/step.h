#pragma once

#include <cstdint>

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
};

struct Step {
  Step(COMMAND command) : m_command(command) {};
  virtual ~Step() {};

  COMMAND m_command;
};

struct SpawnStep : public Step {
  SpawnStep(COMMAND command) : Step(command) {};
  uint32_t m_entity_type;
  sf::Vector3i m_location;
};

struct ColonizeStep : public Step {
  ColonizeStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
  sf::Vector3i m_location;
};
