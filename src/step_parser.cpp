#include "step_parser.h"

#include "step.h"
#include "util.h"
#include "format.h"
#include "game_types.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace {

#define CHECK(arg_count, tokens) { \
  if (tokens.size() < arg_count) { \
    bad_arguments(tokens); \
    return; \
  } \
}

#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return; \
  } \
}

#define CREATE_GENERIC_STEP(arg_count, tokens, step, COMMAND) { \
  CHECK_VALID(arg_count, tokens); \
  step = new Step(COMMAND); \
  return; \
}

  uint32_t s_active_player = 0;
  void parse_tokens(const std::vector<std::string>& tokens, Step*& step);
  void bad_arguments(const std::vector<std::string>& tokens);

  void parse_tokens(const std::vector<std::string>& tokens, Step*& step) {
    if (!tokens.size()) {
      return;
    }

    if (tokens[0] == "quit") {
      CREATE_GENERIC_STEP(1, tokens, step, COMMAND::QUIT);
    }

    else if (tokens[0] == "begin_turn") {
      CHECK_VALID(1, tokens);
      BeginStep* begin_step = new BeginStep(COMMAND::BEGIN_TURN);
      begin_step->m_active_player = s_active_player;
      step = begin_step;
    }

    else if (tokens[0] == "end_turn") {
      CHECK_VALID(1, tokens);
      EndTurnStep* end_turn_step = new EndTurnStep(COMMAND::END_TURN);
      end_turn_step->m_player = s_active_player;
      step = end_turn_step;
      if (player::get_count()) {
        ++s_active_player;
        s_active_player = s_active_player % player::get_count();
      }
      end_turn_step->m_next_player = s_active_player;
    }

    else if (tokens[0] == "active_player") {
      CHECK_VALID(2, tokens);
      s_active_player = std::stoul(tokens[1]);
    }
    else if (tokens[0] == "abort") {
      AbortStep* abort_step = new AbortStep();
      abort_step->m_player = s_active_player;
      abort_step->m_city = std::stoul(tokens[1]);
      abort_step->m_index = std::stoul(tokens[2]);
      step = abort_step;
    }
    else if (tokens[0] == "attack") {
      CHECK_VALID(3, tokens);
      AttackStep* attack_step = new AttackStep(COMMAND::ATTACK);
      attack_step->m_attacker_id = std::stoul(tokens[1]);
      attack_step->m_defender_id = std::stoul(tokens[2]);
      attack_step->m_player = s_active_player;
      step = attack_step;
    }

    else if (tokens[0] == "barbarians") {
      CHECK_VALID(1, tokens);
      BarbarianStep* barbarian_step = new BarbarianStep(COMMAND::BARBARIAN_TURN);
      barbarian_step->m_player = s_active_player;
      step = barbarian_step;
    }

    else if (tokens[0] == "city_defense") {
      CHECK_VALID(2, tokens);
      CityDefenseStep* city_defense_step = new CityDefenseStep(COMMAND::CITY_DEFENSE);
      city_defense_step->m_player = s_active_player;
      city_defense_step->m_unit = std::stoul(tokens[1]);
      step = city_defense_step;
    }
    else if (tokens[0] == "colonize") {
      CHECK(4, tokens);
      ColonizeStep* colonize_step = new ColonizeStep(COMMAND::COLONIZE);
      colonize_step->m_location = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      step = colonize_step;
      colonize_step->m_player = s_active_player;
    }

    else if (tokens[0] == "construct") {
      CHECK(3, tokens);
      ConstructionStep* construction_step = new ConstructionStep(COMMAND::CONSTRUCT);
      construction_step->m_city_id = std::stoul(tokens[1]);
      if (std::isdigit(tokens[2][0])) {
        construction_step->m_production_id = std::stoul(tokens[2]);
      }
      else {
        construction_step->m_production_id = util::enum_to_uint(get_construction_type(tokens[2]));
      }
      construction_step->m_player = s_active_player;
      step = construction_step;
      if (tokens.size() > 3) {
        construction_step->m_cheat = true;
      }
    }

    else if (tokens[0] == "tile_cost") {
      CHECK(5, tokens);
      TileMutatorStep* tile_mutator_step = new TileMutatorStep(COMMAND::TILE_MUTATOR);
      tile_mutator_step->m_destination = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      tile_mutator_step->m_movement_cost = std::stoul(tokens[4]);
      step = tile_mutator_step;
    }

    else if (tokens[0] == "tile_resource") {
      CHECK(5, tokens);
      ResourceMutatorStep* resource_mutator_step = new ResourceMutatorStep(COMMAND::RESOURCE_MUTATOR);
      if (std::isdigit(tokens[1][0])) {
        resource_mutator_step->m_type = std::stoul(tokens[1]);
      }
      else {
        resource_mutator_step->m_type = util::enum_to_uint(get_resource_type(tokens[1]));
      }
      resource_mutator_step->m_destination = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      if (tokens.size() == 6) {
        resource_mutator_step->m_quantity = std::stoul(tokens[5]);
      }
      else {
        resource_mutator_step->m_quantity = 1;
      }
      step = resource_mutator_step;
    }

    else if (tokens[0] == "grant") {
      CHECK_VALID(2, tokens);
      GrantStep* grant_step = new GrantStep();
      grant_step->m_player = s_active_player;
      grant_step->m_science = static_cast<uint32_t>(get_science_type(tokens[1]));
      if (grant_step->m_science == 0) {
        grant_step->m_science = std::stoul(tokens[1]);
      }
      step = grant_step;
    }
    
    else if (tokens[0] == "harvest") {
      CHECK(4, tokens);
      HarvestStep* harvest_step = new HarvestStep(COMMAND::HARVEST);
      harvest_step->m_destination = util::str_to_vector3(tokens[1], tokens[2], tokens[3]);
      harvest_step->m_player = s_active_player;
      step = harvest_step;
    }
    else if (tokens[0] == "improve") {
      CHECK(5, tokens);
      ImproveStep* improve_step = new ImproveStep(COMMAND::IMPROVE);
      if (std::isdigit(tokens[1][0])) {
        improve_step->m_improvement_type = std::stoul(tokens[1]);
      }
      else {
        improve_step->m_improvement_type = util::enum_to_uint(get_improvement_type(tokens[1]));
      }
      improve_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      improve_step->m_player = s_active_player;
      step = improve_step;
    }

    else if (tokens[0] == "join") {
      CHECK(2, tokens);
      AddPlayerStep* player_step = new AddPlayerStep(COMMAND::ADD_PLAYER);
      player_step->m_name = tokens[1];
      if (tokens.size() == 3) {
        if (std::isdigit(tokens[2][0])) {
          player_step->ai_type = util::uint_to_enum<AI_TYPE>(std::stoul(tokens[2]));
        }
        else {
          player_step->ai_type = get_ai_type(tokens[2]);
        }
      }
      else {
        player_step->ai_type = AI_TYPE::HUMAN;
      }
      step = player_step;
    }

    else if (tokens[0] == "kill") {
      CHECK_VALID(2, tokens);
      KillStep* kill_step = new KillStep(COMMAND::KILL);
      kill_step->m_unit_id = std::stoul(tokens[1]);
      step = kill_step;
    }

    else if (tokens[0] == "move") {
      CHECK_VALID(5, tokens);
      MoveStep* move_step = new MoveStep(COMMAND::MOVE);
      move_step->m_unit_id = std::stoul(tokens[1]);
      move_step->m_destination = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      move_step->m_player = s_active_player;
      step = move_step;
    }
    else if (tokens[0] == "pillage") {
      CHECK_VALID(2, tokens);
      PillageStep* pillage_step = new PillageStep(COMMAND::PILLAGE);
      pillage_step->m_player = s_active_player;
      pillage_step->m_unit = std::stoul(tokens[1]);
      step = pillage_step;
    }
    else if (tokens[0] == "queue_move") {
      CHECK_VALID(5, tokens);
      MoveStep* move_step = new MoveStep(COMMAND::QUEUE_MOVE);
      move_step->m_unit_id = std::stoul(tokens[1]);
      move_step->m_destination = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      move_step->m_player = s_active_player;
      step = move_step;
    }

    else if (tokens[0] == "purchase") {
      CHECK(2, tokens);
      PurchaseStep* purchase_step = new PurchaseStep(COMMAND::PURCHASE);
      purchase_step->m_player = s_active_player;
      purchase_step->m_city = std::stoul(tokens[1]);
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          purchase_step->m_production_id = std::stoul(tokens[2]);
        }
        else {
          purchase_step->m_production_id = util::enum_to_uint(get_construction_type(tokens[2]));
        }
      }
      step = purchase_step;
    }

    else if (tokens[0] == "research") {
      CHECK_VALID(2, tokens);
      ResearchStep* research_step = new ResearchStep();
      research_step->m_player = s_active_player;
      research_step->m_science = static_cast<uint32_t>(get_science_type(tokens[1]));
      if (research_step->m_science == 0) {
        research_step->m_science = std::stoul(tokens[1]);
      }
      step = research_step;
    }
    
    else if (tokens[0] == "sell") {
      CHECK(2, tokens);
      SellStep* sell_step = new SellStep(COMMAND::SELL);
      sell_step->m_player = s_active_player;
      sell_step->m_city = std::stoul(tokens[1]);
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          sell_step->m_production_id = std::stoul(tokens[2]);
        }
        else {
          sell_step->m_production_id = util::enum_to_uint(get_construction_type(tokens[2]));
        }
      }
      step = sell_step;
    }

    else if (tokens[0] == "siege") {
      CHECK_VALID(3, tokens);
      SiegeStep* siege_step = new SiegeStep();
      siege_step->m_player = s_active_player;
      siege_step->m_city = std::stoul(tokens[1]);
      siege_step->m_unit = std::stoul(tokens[2]);
      step = siege_step;
    }
    else if (tokens[0] == "specialize") {
      CHECK(3, tokens);
      SpecializeStep* specialize_step = new SpecializeStep(COMMAND::SPECIALIZE);
      specialize_step->m_city_id = std::stoul(tokens[1]);
      if (std::isdigit(tokens[2][0])) {
        specialize_step->m_terrain_type = std::stoul(tokens[2]);
      }
      else {
        specialize_step->m_terrain_type = util::enum_to_uint(get_terrain_type(tokens[2]));
      }
      specialize_step->m_player = s_active_player;
      step = specialize_step;
    }

    else if (tokens[0] == "spawn") {
      CHECK(5, tokens);
      step = new SpawnStep(COMMAND::SPAWN);
      SpawnStep* spawn_step = static_cast<SpawnStep*>(step);
      // If first character in string is a number treat it as an id.
      if (std::isdigit(tokens[1][0])) {
        spawn_step->m_unit_type = std::stoul(tokens[1]);
      }
      // Else treat it as a the name of what needs to be spawned.
      else {
        spawn_step->m_unit_type = util::enum_to_uint(get_unit_type(tokens[1]));
      }
      spawn_step->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      // Optional player param, defaults to 0, or the 'player one'
      spawn_step->m_player = s_active_player;
    }

    else if (tokens[0] == "stats") {
      CHECK_VALID(5, tokens);
      UnitStatsStep* stats = new UnitStatsStep(COMMAND::MODIFY_UNIT_STATS);
      stats->m_unit_id = std::stoul(tokens[1]);
      stats->m_health = std::stoul(tokens[2]);
      stats->m_attack = std::stoul(tokens[3]);
      stats->m_range = std::stoul(tokens[4]);
      step = stats;
    }

    else if (tokens[0] == "cast") {
      CHECK(5, tokens);
      MagicStep* magic = new MagicStep();
      magic->m_type = get_magic_type(tokens[1]);
      magic->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      magic->m_player = s_active_player;
      if (tokens.size() > 5) {
        magic->m_cheat = true;
      }
      step = magic;
    }

    else if (tokens[0] == "status") {
      CHECK(5, tokens);
      StatusStep* status = new StatusStep();
      status->m_type = get_status_type(tokens[1]);
      status->m_location = util::str_to_vector3(tokens[2], tokens[3], tokens[4]);
      step = status;
    }

    else {
      std::cout << "Unrecognized step: " << format::vector(tokens) << std::endl;
    }
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::vector(tokens) << std::endl;
  }
}

void step_parser::split_to_tokens(const std::string& line, std::vector<std::string>& tokens) {
  std::stringstream iss(line);
  // Make sure output vector is empty first
  tokens.clear();

  // Split string on space
  std::copy(std::istream_iterator<std::string>(iss),
    std::istream_iterator<std::string>(),
    std::back_inserter(tokens));
}

Step* step_parser::parse(const std::vector<std::string>& tokens) {
  Step* step = nullptr;
  try
  {
    parse_tokens(tokens, step);
  }
  catch(const std::exception&e ) 
  {
    std::cout << "Exception thrown: " << e.what() << std::endl;
  }
  return step;
}

std::string step_parser::get_active_player() {
  Player* player = player::get_player(s_active_player);
  if (!player) {
    return std::string();
  }
  std::stringstream ss;
  ss << player->m_name << " (gold " << player->m_gold << ") (science " << player->m_science << ") (magic " << player->m_magic << ")";
  return ss.str();
}

uint32_t step_parser::get_active_player_id() {
  return s_active_player;
}

