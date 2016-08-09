#include "step_parser.h"

#include "util.h"
#include "format.h"
#include "network_types.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace {

#define CHECK(arg_count, tokens) { \
  if (tokens.size() < arg_count) { \
    bad_arguments(tokens); \
    return 0; \
  } \
}

#define CHECK_VALID(arg_count, tokens) { \
  if (tokens.size() != arg_count) { \
    bad_arguments(tokens); \
    return 0; \
  } \
}

  uint32_t s_active_player = 0;
  size_t parse_tokens(const std::vector<std::string>& tokens, NETWORK_TYPE& operation, void* buffer, size_t buffer_len);
  void bad_arguments(const std::vector<std::string>& tokens);

  size_t parse_tokens(const std::vector<std::string>& tokens, NETWORK_TYPE& operation, void* buffer, size_t buffer_len) {
    operation = NETWORK_TYPE::UNKNOWN;
    if (!tokens.size()) {
      return 0;
    }
    size_t bytes_written = 0;

    if (tokens[0] == "quit") {
      QuitStep quit;
      bytes_written = serialize(buffer, buffer_len, quit);
    }

    else if (tokens[0] == "begin_turn") {
      CHECK_VALID(1, tokens);
      BeginStep begin_step;
      begin_step.set_active_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, begin_step);
    }

    else if (tokens[0] == "end_turn") {
      CHECK_VALID(1, tokens);
      EndTurnStep end_turn_step;
      end_turn_step.set_player(s_active_player);
      end_turn_step.set_next_player(s_active_player);
      if (player::get_count()) {
        ++s_active_player;
        s_active_player = s_active_player % player::get_count();
      }
      bytes_written = serialize(buffer, buffer_len, end_turn_step);
    }

    else if (tokens[0] == "active_player") {
      CHECK_VALID(2, tokens);
      s_active_player = std::stoul(tokens[1]);
    }
    else if (tokens[0] == "production_abort") {
      ProductionAbortStep abort_step;
      abort_step.set_player(s_active_player);
      abort_step.set_city(std::stoul(tokens[1]));
      abort_step.set_index(std::stoul(tokens[2]));
      bytes_written = serialize(buffer, buffer_len, abort_step);
    }
    else if (tokens[0] == "production_move") {
      ProductionMoveStep move_step;
      move_step.set_player(s_active_player);
      move_step.set_city(std::stoul(tokens[1]));
      move_step.set_source_index(std::stoul(tokens[2]));
      move_step.set_destination_index(std::stoul(tokens[3]));
      bytes_written = serialize(buffer, buffer_len, move_step);
    }
    else if (tokens[0] == "attack") {
      CHECK_VALID(3, tokens);
      AttackStep attack_step;
      attack_step.set_attacker_id(std::stoul(tokens[1]));
      attack_step.set_defender_id(std::stoul(tokens[2]));
      attack_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, attack_step);
    }

    else if (tokens[0] == "barbarians") {
      CHECK_VALID(1, tokens);
      BarbarianStep barbarian_step;
      barbarian_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, barbarian_step);
    }

    else if (tokens[0] == "city_defense") {
      CHECK_VALID(2, tokens);
      CityDefenseStep city_defense_step;
      city_defense_step.set_player(s_active_player);
      city_defense_step.set_unit(std::stoul(tokens[1]));
      bytes_written = serialize(buffer, buffer_len, city_defense_step);
    }
    else if (tokens[0] == "colonize") {
      CHECK(4, tokens);
      ColonizeStep colonize_step;
      colonize_step.set_location(util::str_to_vector3(tokens[1], tokens[2], tokens[3]));
      colonize_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, colonize_step);
    }
    else if (tokens[0] == "construct") {
      CHECK(3, tokens);
      ConstructionStep construction_step;
      construction_step.set_city_id(std::stoul(tokens[1]));
      if (std::isdigit(tokens[2][0])) {
        construction_step.set_production_id(std::stoul(tokens[2]));
      }
      else {
        construction_step.set_production_id(util::enum_to_uint(get_construction_type(tokens[2])));
      }
      construction_step.set_player(s_active_player);
      if (tokens.size() > 3) {
        construction_step.set_cheat(true);
      }
      bytes_written = serialize(buffer, buffer_len, construction_step);
    }

    else if (tokens[0] == "tile_cost") {
      CHECK(5, tokens);
      TileMutatorStep tile_mutator;
      tile_mutator.set_destination(util::str_to_vector3(tokens[1], tokens[2], tokens[3]));
      tile_mutator.set_movement_cost(std::stoul(tokens[4]));
      bytes_written = serialize(buffer, buffer_len, tile_mutator);
    }

    else if (tokens[0] == "tile_resource") {
      CHECK(5, tokens);
      ResourceMutatorStep resource_mutator;
      
      ResourceMutatorStep* resource_mutator_step = new ResourceMutatorStep();
      if (std::isdigit(tokens[1][0])) {
        resource_mutator.set_type(std::stoul(tokens[1]));
      }
      else {
        resource_mutator.set_type(util::enum_to_uint(get_resource_type(tokens[1])));
      }
      resource_mutator.set_destination(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      resource_mutator.set_quantity(1);
      if (tokens.size() == 6) {
        resource_mutator.set_quantity(std::stoul(tokens[5]));
      }
      bytes_written = serialize(buffer, buffer_len, resource_mutator);
    }

    else if (tokens[0] == "grant") {
      CHECK_VALID(2, tokens);
      GrantStep grant_step;

      grant_step.set_player(s_active_player);
      grant_step.set_science(static_cast<uint32_t>(get_science_type(tokens[1])));
      if (grant_step.get_science() == 0) {
        grant_step.set_science(std::stoul(tokens[1]));
      }
      bytes_written = serialize(buffer, buffer_len, grant_step);
    }
    
    else if (tokens[0] == "harvest") {
      CHECK(4, tokens);
      HarvestStep harvest_step;
      harvest_step.set_destination(util::str_to_vector3(tokens[1], tokens[2], tokens[3]));
      harvest_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, harvest_step);
    }
    else if (tokens[0] == "improve") {
      CHECK(6, tokens);
      ImproveStep improve_step;
      if (std::isdigit(tokens[1][0])) {
        improve_step.set_resource(std::stoul(tokens[1]));
      }
      else {
        improve_step.set_resource(util::enum_to_uint(get_resource_type(tokens[1])));
      }
      if (std::isdigit(tokens[2][0])) {
        improve_step.set_improvement_type(std::stoul(tokens[2]));
      }
      else {
        improve_step.set_improvement_type(util::enum_to_uint(get_improvement_type(tokens[2])));
      }
      improve_step.set_location(util::str_to_vector3(tokens[3], tokens[4], tokens[5]));
      improve_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, improve_step);
    }

    else if (tokens[0] == "join") {
      CHECK(2, tokens);
      AddPlayerStep player_step;
      player_step.set_name(tokens[1]);
      player_step.set_ai_type(AI_TYPE::HUMAN);
      if (tokens.size() == 3) {
        if (std::isdigit(tokens[2][0])) {
          player_step.set_ai_type(util::uint_to_enum<AI_TYPE>(std::stoul(tokens[2])));
        }
        else {
          player_step.set_ai_type(get_ai_type(tokens[2]));
        }
      }
      bytes_written = serialize(buffer, buffer_len, player_step);
    }

    else if (tokens[0] == "kill") {
      CHECK_VALID(2, tokens);
      KillStep kill_step;
      kill_step.set_unit_id(std::stoul(tokens[1]));
      bytes_written = serialize(buffer, buffer_len, kill_step);
    }

    else if (tokens[0] == "move") {
      CHECK_VALID(5, tokens);
      MoveStep move_step;
      move_step.set_unit_id(std::stoul(tokens[1]));
      move_step.set_destination(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      move_step.set_player(s_active_player);
      move_step.set_immediate(true);
      bytes_written = serialize(buffer, buffer_len, move_step);
    }
    else if (tokens[0] == "pillage") {
      CHECK_VALID(2, tokens);
      PillageStep pillage_step;
      pillage_step.set_player(s_active_player);
      pillage_step.set_unit(std::stoul(tokens[1]));
      bytes_written = serialize(buffer, buffer_len, pillage_step);
    }
    else if (tokens[0] == "queue_move") {
      CHECK_VALID(5, tokens);
      MoveStep move_step;
      move_step.set_unit_id(std::stoul(tokens[1]));
      move_step.set_destination(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      move_step.set_player(s_active_player);
      move_step.set_immediate(false);
      bytes_written = serialize(buffer, buffer_len, move_step);
    }

    else if (tokens[0] == "purchase") {
      CHECK(2, tokens);
      PurchaseStep purchase_step;
      purchase_step.set_player(s_active_player);
      purchase_step.set_city(std::stoul(tokens[1]));
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          purchase_step.set_production(std::stoul(tokens[2]));
        }
        else {
          purchase_step.set_production(util::enum_to_uint(get_construction_type(tokens[2])));
        }
      }
      bytes_written = serialize(buffer, buffer_len, purchase_step);
    }

    else if (tokens[0] == "research") {
      CHECK_VALID(2, tokens);
      ResearchStep research_step;
      research_step.set_player(s_active_player);
      research_step.set_science(static_cast<uint32_t>(get_science_type(tokens[1])));
      if (research_step.get_science() == 0) {
        research_step.set_science(std::stoul(tokens[1]));
      }
      bytes_written = serialize(buffer, buffer_len, research_step);
    }
    
    else if (tokens[0] == "sell") {
      CHECK(2, tokens);
      SellStep sell_step;
      sell_step.set_city(std::stoul(tokens[1]));
      sell_step.set_player(s_active_player);
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          sell_step.set_production_id(std::stoul(tokens[2]));
        }
        else {
          sell_step.set_production_id(util::enum_to_uint(get_construction_type(tokens[2])));
        }
      }
      bytes_written = serialize(buffer, buffer_len, sell_step);
    }

    else if (tokens[0] == "siege") {
      CHECK_VALID(3, tokens);
      SiegeStep siege_step;
      siege_step.set_city(std::stoul(tokens[1]));
      siege_step.set_unit(std::stoul(tokens[2]));
      siege_step.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, siege_step);
    }
    else if (tokens[0] == "specialize") {
      CHECK(3, tokens);
      SpecializeStep specialize_step;
      specialize_step.set_player(s_active_player);
      specialize_step.set_city_id(std::stoul(tokens[1]));
      if (std::isdigit(tokens[2][0])) {
        specialize_step.set_terrain_type(std::stoul(tokens[2]));
      }
      else {
        specialize_step.set_terrain_type(util::enum_to_uint(get_terrain_type(tokens[2])));
      }
      bytes_written = serialize(buffer, buffer_len, specialize_step);
    }

    else if (tokens[0] == "spawn") {
      CHECK(5, tokens);
      SpawnStep spawn;

      // If first character in string is a number treat it as an id.
      if (std::isdigit(tokens[1][0])) {
        spawn.set_unit_type(std::stoul(tokens[1]));
      }
      // Else treat it as a the name of what needs to be spawned.
      else {
        spawn.set_unit_type(util::enum_to_uint(get_unit_type(tokens[1])));
      }
      spawn.set_location(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      spawn.set_player(s_active_player);
      bytes_written = serialize(buffer, buffer_len, spawn);
    }

    else if (tokens[0] == "stats") {
      CHECK_VALID(5, tokens);
      UnitStatsStep stats;
      stats.set_unit_id(std::stoul(tokens[1]));
      stats.set_health(std::stoul(tokens[2]));
      stats.set_attack(std::stoul(tokens[3]));
      stats.set_range(std::stoul(tokens[4]));
      bytes_written = serialize(buffer, buffer_len, stats);
    }

    else if (tokens[0] == "cast") {
      CHECK(5, tokens);
      MagicStep magic;
      magic.set_type(get_magic_type(tokens[1]));
      magic.set_location(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      magic.set_player(s_active_player);
      if (tokens.size() > 5) {
        magic.set_cheat(true);
      }
      bytes_written = serialize(buffer, buffer_len, magic);
    }

    else if (tokens[0] == "status") {
      CHECK(5, tokens);
      StatusStep status;
      status.set_type(get_status_type(tokens[1]));
      status.set_location(util::str_to_vector3(tokens[2], tokens[3], tokens[4]));
      bytes_written = serialize(buffer, buffer_len, status);
    }

    else {
      std::cout << "Unrecognized step: " << format::vector(tokens) << std::endl;
      return 0;
    }

    if (bytes_written == 0) {
      std::cout << "Failure to serialize step" << std::endl;
      return 0;
    }

    operation = read_type(buffer, bytes_written);
    return bytes_written;
  }

  void bad_arguments(const std::vector<std::string>& tokens) {
    std::cout << "Invalid arguments: " << format::vector(tokens) << std::endl;
  }
}

std::vector<std::string> step_parser::split_to_tokens(const std::string& line) {
  std::vector<std::string> tokens;

  std::stringstream iss(line);
  // Make sure output vector is empty first
  tokens.clear();

  // Split string on space
  std::copy(std::istream_iterator<std::string>(iss),
    std::istream_iterator<std::string>(),
    std::back_inserter(tokens));

  return std::move(tokens);
}

size_t step_parser::parse(const std::vector<std::string>& tokens, NETWORK_TYPE& operation, void* buffer, size_t buffer_len) {
  try
  {
    return parse_tokens(tokens, operation, buffer, buffer_len);
  }
  catch(const std::exception&e ) 
  {
    std::cout << "Exception thrown: " << e.what() << std::endl;
  }
  
  return 0;
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

void step_parser::set_active_player_id(uint32_t player_id) {
  s_active_player = player_id;
}

