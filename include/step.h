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
  QUEUE_MOVE,
  PURCHASE,
  SELL,
  TILE_MUTATOR,
  RESOURCE_MUTATOR,
  SPAWN,
  ADD_PLAYER,
  MODIFY_UNIT_STATS,
  HARVEST,
};

struct Step {
  Step(COMMAND command) : m_command(command) {};
  virtual ~Step() {};

  COMMAND m_command;
};

struct SpawnStep : public Step {
  SpawnStep(COMMAND command) : Step(command)
    , m_unit_type(0)
    , m_location()
    , m_player(0) {};

  uint32_t m_unit_type;
  sf::Vector3i m_location;
  uint32_t m_player;
};

struct ImproveStep : public Step {
  ImproveStep(COMMAND command) : Step(command)
    , m_improvement_type(0)
    , m_location()
    , m_player(0) {};

  uint32_t m_improvement_type;
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

struct ConstructionStep : public Step {
  ConstructionStep(COMMAND command) : Step(command)
    , m_city_id(0)
    , m_production_id(0)
    , m_cheat(false)
    , m_player(0)
    {};

  uint32_t m_city_id;
  uint32_t m_production_id;
  bool m_cheat;
  uint32_t m_player;
};

struct MoveStep : public Step {
  MoveStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
  sf::Vector3i m_destination;
  uint32_t m_player;
};

struct QueueMoveStep : public Step {
  QueueMoveStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
  sf::Vector3i m_destination;
  uint32_t m_player;
};

struct AddPlayerStep : public Step {
  AddPlayerStep(COMMAND command) : Step(command) {};
  std::string m_name;
};

struct AttackStep : public Step {
  AttackStep(COMMAND command) : Step(command) {};
  uint32_t m_attacker_id;
  uint32_t m_defender_id;
  uint32_t m_player;
};

struct KillStep : public Step {
  KillStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
};

struct UnitStatsStep : public Step {
  UnitStatsStep(COMMAND command) : Step(command) {};
  uint32_t m_unit_id;
  uint32_t m_health;
  uint32_t m_attack; 
  uint32_t m_range;
};

struct TileMutatorStep : public Step {
  TileMutatorStep(COMMAND command) : Step(command) {};
  sf::Vector3i m_destination;
  uint32_t m_movement_cost;
};

struct ResourceMutatorStep : public Step {
  ResourceMutatorStep(COMMAND command) : Step(command) {};
  sf::Vector3i m_destination;
  uint32_t m_type;
  uint32_t m_quantity;
};

struct EndTurnStep : public Step {
  EndTurnStep(COMMAND command) : Step(command) {};

  uint32_t m_player;
};

struct HarvestStep : public Step {
  HarvestStep(COMMAND command) : Step(command) {};

  uint32_t m_player;
  sf::Vector3i m_destination;
};

