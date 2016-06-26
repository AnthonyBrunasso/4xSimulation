#pragma once

#include <cstdint>
#include <string>

#include "game_types.h"
#include "Vector3.hpp"

struct Step {
  Step(COMMAND_TYPE command) : m_command(command) {};
  virtual ~Step() {};

  COMMAND_TYPE m_command;
};

struct SpawnStep : public Step {
  SpawnStep() : Step(COMMAND_TYPE::SPAWN)
    , m_unit_type(0)
    , m_location()
    , m_player(0) {};

  uint32_t m_unit_type;
  sf::Vector3i m_location;
  uint32_t m_player;
};

struct ImproveStep : public Step {
  ImproveStep() : Step(COMMAND_TYPE::IMPROVE)
    , m_improvement_type(0)
    , m_location()
    , m_player(0) {};

  uint32_t m_improvement_type;
  sf::Vector3i m_location;
  uint32_t m_resource;
  uint32_t m_player;
};

struct ColonizeStep : public Step {
  ColonizeStep() : Step(COMMAND_TYPE::COLONIZE)
    , m_location()
    , m_player(0) {};
    
  sf::Vector3i m_location;
  uint32_t m_player;
};

struct ConstructionStep : public Step {
  ConstructionStep() : Step(COMMAND_TYPE::CONSTRUCT)
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
  MoveStep() : Step(COMMAND_TYPE::MOVE) {};
  uint32_t m_unit_id;
  sf::Vector3i m_destination;
  uint32_t m_player;
};

struct PurchaseStep : public Step {
  PurchaseStep() 
  : Step(COMMAND_TYPE::PURCHASE) 
  , m_player(0)
  , m_city(0)
  , m_production_id(0)
  {};

  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_production_id;
};

struct SellStep : public Step {
  SellStep()
  : Step(COMMAND_TYPE::SELL)
  , m_player(0)
  , m_city(0)
  , m_production_id(0)
  {};

  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_production_id;
};


struct AddPlayerStep : public Step {
  AddPlayerStep() : Step(COMMAND_TYPE::ADD_PLAYER), m_name(), ai_type(AI_TYPE::UNKNOWN) {};
  std::string m_name;
  AI_TYPE ai_type; 
};

struct AttackStep : public Step {
  AttackStep() : Step(COMMAND_TYPE::ATTACK) {};
  uint32_t m_attacker_id;
  uint32_t m_defender_id;
  uint32_t m_player;
};

struct KillStep : public Step {
  KillStep() : Step(COMMAND_TYPE::KILL) {};
  uint32_t m_unit_id;
};

struct UnitStatsStep : public Step {
  UnitStatsStep() : Step(COMMAND_TYPE::MODIFY_UNIT_STATS) {};
  uint32_t m_unit_id;
  uint32_t m_health;
  uint32_t m_attack; 
  uint32_t m_range;
};

struct TileMutatorStep : public Step {
  TileMutatorStep() : Step(COMMAND_TYPE::TILE_MUTATOR) {};
  sf::Vector3i m_destination;
  uint32_t m_movement_cost;
};

struct ResourceMutatorStep : public Step {
  ResourceMutatorStep() : Step(COMMAND_TYPE::RESOURCE_MUTATOR) {};
  sf::Vector3i m_destination;
  uint32_t m_type;
  uint32_t m_quantity;
};

struct EndTurnStep : public Step {
  EndTurnStep() : Step(COMMAND_TYPE::END_TURN) {};

  uint32_t m_player;
  uint32_t m_next_player;
};

struct HarvestStep : public Step {
  HarvestStep() : Step(COMMAND_TYPE::HARVEST) {};

  uint32_t m_player;
  sf::Vector3i m_destination;
};

struct SpecializeStep : public Step {
  SpecializeStep() : Step(COMMAND_TYPE::SPECIALIZE) {};

  uint32_t m_city_id;
  uint32_t m_terrain_type;
  uint32_t m_player;
};

struct BarbarianStep : public Step {
  BarbarianStep() : Step(COMMAND_TYPE::BARBARIAN_TURN) {};

  uint32_t m_player;
};

struct BeginStep : public Step {
  BeginStep() : Step(COMMAND_TYPE::BEGIN_TURN) {};
  uint32_t m_active_player;
};

struct CityDefenseStep : public Step {
  CityDefenseStep() : Step(COMMAND_TYPE::CITY_DEFENSE) {}
  uint32_t m_player;
  uint32_t m_unit;
};

struct PillageStep : public Step {
  PillageStep() : Step(COMMAND_TYPE::PILLAGE) {}

  uint32_t m_player;
  uint32_t m_unit;
};

struct AbortStep : public Step {
  AbortStep() : Step(COMMAND_TYPE::ABORT) {}
  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_index;
};

struct SiegeStep : public Step {
  SiegeStep() : Step(COMMAND_TYPE::SIEGE) {}
  uint32_t m_player;
  uint32_t m_city;
  uint32_t m_unit;
};

struct GrantStep : public Step {
  GrantStep() : Step(COMMAND_TYPE::GRANT) {}
  uint32_t m_player;
  uint32_t m_science;
};

struct MagicStep : public Step {
  MagicStep() : Step(COMMAND_TYPE::MAGIC), m_cheat(false) {};
  uint32_t m_player;
  sf::Vector3i m_location;
  MAGIC_TYPE m_type;
  bool m_cheat;
};

struct ResearchStep : public Step {
  ResearchStep() : Step(COMMAND_TYPE::RESEARCH) {};
  uint32_t m_player;
  uint32_t m_science;
};

struct StatusStep : public Step {
  StatusStep() : Step(COMMAND_TYPE::STATUS) {};
  STATUS_TYPE m_type;
  sf::Vector3i m_location;
};

