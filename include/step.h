#pragma once

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
};

struct Step {
  Step(COMMAND command) :
    m_command(command) {};

  COMMAND m_command;
};