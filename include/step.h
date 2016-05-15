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

  COMMAND m_command;
};

struct SpawnStep : public Step {
  SpawnStep(COMMAND command) : Step(command) {};
  uint32_t m_uintId;
  sf::Vector3i m_location;
};