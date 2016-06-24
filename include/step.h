#pragma once

#include <cstdint>
#include <string>

#include "game_types.h"
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
  SPECIALIZE,
  BARBARIAN_TURN,
  CITY_DEFENSE,
  PILLAGE,
  ABORT,
  SIEGE,
  GRANT,
  MAGIC,
  RESEARCH,
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
    , m_location()
    , m_player(0) {};
    
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

struct PurchaseStep : public Step {
  PurchaseStep(COMMAND command) 
  : Step(command) 
  , m_player(0)
  , m_city(0)
  , m_production_id(0)
  {};

  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_production_id;
};

struct SellStep : public Step {
  SellStep(COMMAND command)
  : Step(command)
  , m_player(0)
  , m_city(0)
  , m_production_id(0)
  {};

  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_production_id;
};


struct AddPlayerStep : public Step {
  AddPlayerStep(COMMAND command) : Step(command), m_name(), ai_type(AI_TYPE::UNKNOWN) {};
  std::string m_name;
  AI_TYPE ai_type; 
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
  uint32_t m_next_player;
};

struct HarvestStep : public Step {
  HarvestStep(COMMAND command) : Step(command) {};

  uint32_t m_player;
  sf::Vector3i m_destination;
};

struct SpecializeStep : public Step {
  SpecializeStep(COMMAND command) : Step(command) {};

  uint32_t m_city_id;
  uint32_t m_terrain_type;
  uint32_t m_player;
};

struct BarbarianStep : public Step {
  BarbarianStep(COMMAND command) : Step(command) {};

  uint32_t m_player;
};

struct BeginStep : public Step {
  BeginStep(COMMAND command) : Step(command) {};
  uint32_t m_active_player;
};

struct CityDefenseStep : public Step {
  CityDefenseStep(COMMAND command) : Step(command) {}
  uint32_t m_player;
  uint32_t m_unit;
};

struct PillageStep : public Step {
  PillageStep(COMMAND command) : Step(command) {}

  uint32_t m_player;
  uint32_t m_unit;
};

struct AbortStep : public Step {
  AbortStep() : Step(COMMAND::ABORT) {}
  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_index;
};

struct SiegeStep : public Step {
  SiegeStep() : Step(COMMAND::SIEGE) {}
  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_unit;
};

struct GrantStep : public Step {
  GrantStep() : Step(COMMAND::GRANT) {}
  uint32_t m_player;
  uint32_t m_science;
};

struct MagicStep : public Step {
  MagicStep() : Step(COMMAND::MAGIC), m_cheat(false) {};
  uint32_t m_player;
  sf::Vector3i m_location;
  MAGIC_TYPE m_type;
  bool m_cheat;
};

struct ResearchStep : public Step {
  ResearchStep() : Step(COMMAND::RESEARCH) {};
  uint32_t m_player;
  uint32_t m_science;
};

