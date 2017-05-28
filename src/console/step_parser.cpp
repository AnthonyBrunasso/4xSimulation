#include "step_parser.h"

#include <ext/alloc_traits.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>

#include "flatbuffers/flatbuffers.h"
#include "format.h"
#include "game_types.h"
#include "player.h"
#include "step_generated.h"
#include "util.h"

namespace step_parser {

  flatbuffers::FlatBufferBuilder& GetFBB() {
    static flatbuffers::FlatBufferBuilder builder;
    return builder;
  }

  void FBBToBuffer(void* buffer) {
      std::memcpy(buffer, GetFBB().GetBufferPointer(), GetFBB().GetSize());
  }

  fbs::v3i str_to_v3i(const std::string& x_str, const std::string& y_str, const std::string& z_str) {
    fbs::v3i v3i(std::stoi(x_str), std::stoi(y_str), std::stoi(z_str));
    return v3i;
  }

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
  size_t parse_tokens(const std::vector<std::string>& tokens, void* buffer, size_t buffer_len);
  void bad_arguments(const std::vector<std::string>& tokens);

  size_t parse_tokens(const std::vector<std::string>& tokens, void* buffer, size_t buffer_len) {
    GetFBB().Clear();
    if (!tokens.size()) {
      return 0;
    }
    auto copy_to_netbuffer = [buffer, buffer_len] (fbs::StepUnion step_type, const flatbuffers::Offset<void>& step) {
      flatbuffers::Offset<fbs::AnyStep> anystep = fbs::CreateAnyStep(GetFBB(), step_type, step);
      fbs::FinishAnyStepBuffer(GetFBB(), anystep);
      if (GetFBB().GetSize() > buffer_len) {
        std::cout << "FlatBufferBuilder exceeded network buffer!! StepType: " << (uint32_t)step_type << std::endl;
        return;
      }

      FBBToBuffer(buffer);
    };

    if (tokens[0] == "quit") {
      flatbuffers::Offset<fbs::QuitStep> quit = fbs::CreateQuitStep(GetFBB());
      copy_to_netbuffer(fbs::StepUnion::QuitStep, quit.Union());
    }

    else if (tokens[0] == "begin_turn") {
      CHECK_VALID(1, tokens);
      flatbuffers::Offset<fbs::BeginStep> begin = fbs::CreateBeginStep(GetFBB(), s_active_player);
      copy_to_netbuffer(fbs::StepUnion::BeginStep, begin.Union());
    }

    else if (tokens[0] == "end_turn") {
      CHECK_VALID(1, tokens);

      uint32_t current_player = s_active_player;
      if (player::get_count()) {
        ++s_active_player;
        s_active_player = s_active_player % player::get_count();
      }
      uint32_t next_player = s_active_player;

      flatbuffers::Offset<fbs::EndTurnStep> end_turn_step = fbs::CreateEndTurnStep(GetFBB(), current_player, next_player);
      copy_to_netbuffer(fbs::StepUnion::EndTurnStep, end_turn_step.Union());
    }

    else if (tokens[0] == "active_player") {
      CHECK_VALID(2, tokens);
      s_active_player = std::stoul(tokens[1]);
    }
    else if (tokens[0] == "production_abort") {
      uint32_t city_id = std::stoul(tokens[1]);
      uint32_t index = std::stoul(tokens[2]);
      flatbuffers::Offset<fbs::ProductionAbortStep> abort_step = fbs::CreateProductionAbortStep(GetFBB(), s_active_player, city_id, index);
      copy_to_netbuffer(fbs::StepUnion::ProductionAbortStep, abort_step.Union());
    }
    else if (tokens[0] == "production_move") {
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t src_index = (std::stoul(tokens[2]));
      uint32_t dst_index = (std::stoul(tokens[3]));
      flatbuffers::Offset<fbs::ProductionMoveStep> move_step = fbs::CreateProductionMoveStep(GetFBB(), s_active_player, city_id, src_index, dst_index);
      copy_to_netbuffer(fbs::StepUnion::ProductionMoveStep, move_step.Union());
    }
    else if (tokens[0] == "attack") {
      CHECK_VALID(3, tokens);
      uint32_t attacker_id = (std::stoul(tokens[1]));
      uint32_t defender_id = (std::stoul(tokens[2]));
      flatbuffers::Offset<fbs::AttackStep> attack_step = fbs::CreateAttackStep(GetFBB(), attacker_id, defender_id, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::AttackStep, attack_step.Union());
    }

    else if (tokens[0] == "barbarians") {
      CHECK_VALID(1, tokens);
      flatbuffers::Offset<fbs::BarbarianStep> barb_step = fbs::CreateBarbarianStep(GetFBB(), s_active_player);
      copy_to_netbuffer(fbs::StepUnion::BarbarianStep, barb_step.Union());
    }

    else if (tokens[0] == "city_defense") {
      CHECK_VALID(2, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      flatbuffers::Offset<fbs::CityDefenseStep> defense_step = fbs::CreateCityDefenseStep(GetFBB(), s_active_player, unit_id);
      copy_to_netbuffer(fbs::StepUnion::CityDefenseStep, defense_step.Union());
    }
    else if (tokens[0] == "colonize") {
      CHECK(4, tokens);
      fbs::v3i location = str_to_v3i(tokens[1], tokens[2], tokens[3]);
      flatbuffers::Offset<fbs::ColonizeStep> colonize_step = fbs::CreateColonizeStep(GetFBB(), &location, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::ColonizeStep, colonize_step.Union());
    }
    else if (tokens[0] == "construct") {
      CHECK(3, tokens);
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t construction_id = 0;
      bool cheat = false;
      if (std::isdigit(tokens[2][0])) {
        construction_id = (std::stoul(tokens[2]));
      }
      else {
        construction_id = (util::enum_to_uint(get_construction_type(tokens[2])));
      }
      if (tokens.size() > 3) {
        cheat = true;
      }
      flatbuffers::Offset<fbs::ConstructionStep> construction_step = fbs::CreateConstructionStep(GetFBB(), city_id, construction_id, cheat, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::ConstructionStep, construction_step.Union());
    }

    else if (tokens[0] == "tile_cost") {
      CHECK(5, tokens);
      fbs::v3i dest = str_to_v3i(tokens[1], tokens[2], tokens[3]);
      uint32_t movement_cost = (std::stoul(tokens[4]));
      flatbuffers::Offset<fbs::TileMutatorStep> tile_mutator_step = fbs::CreateTileMutatorStep(GetFBB(), &dest, movement_cost);
      copy_to_netbuffer(fbs::StepUnion::TileMutatorStep, tile_mutator_step.Union());
    }

    else if (tokens[0] == "tile_resource") {
      CHECK(5, tokens);
      uint32_t type_id = 0;

      if (std::isdigit(tokens[1][0])) {
        type_id = (std::stoul(tokens[1]));
      }
      else {
        type_id = (util::enum_to_uint(get_resource_type(tokens[1])));
      }

      fbs::v3i dest = (str_to_v3i(tokens[2], tokens[3], tokens[4]));
      uint32_t quantity = 1;
      if (tokens.size() == 6) {
        quantity = (std::stoul(tokens[5]));
      }
      flatbuffers::Offset<fbs::ResourceMutatorStep> resource_mutator = fbs::CreateResourceMutatorStep(GetFBB(), &dest, type_id, quantity);
      copy_to_netbuffer(fbs::StepUnion::ResourceMutatorStep, resource_mutator.Union());
    }

    else if (tokens[0] == "grant") {
      CHECK_VALID(2, tokens);

      uint32_t science_id = (static_cast<uint32_t>(get_science_type(tokens[1])));
      if (science_id == 0) {
        science_id = (std::stoul(tokens[1]));
      }
      flatbuffers::Offset<fbs::GrantStep> grant_step = fbs::CreateGrantStep(GetFBB(), s_active_player, science_id);
      copy_to_netbuffer(fbs::StepUnion::GrantStep, grant_step.Union());
    }
    else if (tokens[0] == "harvest") {
      CHECK(4, tokens);
      fbs::v3i dest = str_to_v3i(tokens[1], tokens[2], tokens[3]);
      flatbuffers::Offset<fbs::HarvestStep> harvest_step = fbs::CreateHarvestStep(GetFBB(), s_active_player, &dest);
      copy_to_netbuffer(fbs::StepUnion::HarvestStep, harvest_step.Union());
    }
    else if (tokens[0] == "improve") {
      CHECK(5, tokens);
      uint32_t resource_id;
      if (std::isdigit(tokens[1][0])) {
        resource_id = std::stoul(tokens[1]);
      }
      else {
        resource_id = (util::enum_to_uint(get_resource_type(tokens[1])));
      }
      fbs::v3i dest = str_to_v3i(tokens[2], tokens[3], tokens[4]);
      flatbuffers::Offset<fbs::ImproveStep> improve_step = fbs::CreateImproveStep(GetFBB(), &dest, resource_id, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::ImproveStep, improve_step.Union());
    }

    else if (tokens[0] == "join") {
      CHECK(2, tokens);
      AI_TYPE ai_type = (AI_TYPE::HUMAN);
      if (tokens.size() == 3) {
        if (std::isdigit(tokens[2][0])) {
          ai_type = util::uint_to_enum<AI_TYPE>(std::stoul(tokens[2]));
        }
        else {
          ai_type = (get_ai_type(tokens[2]));
        }
      }
      flatbuffers::Offset<flatbuffers::String> name = GetFBB().CreateString(tokens[1].c_str(), tokens[1].size());
      flatbuffers::Offset<fbs::AddPlayerStep> add_player_step = fbs::CreateAddPlayerStep(GetFBB(), name, (fbs::AI_TYPE)ai_type); 
      copy_to_netbuffer(fbs::StepUnion::AddPlayerStep, add_player_step.Union()); 
    } 
    else if (tokens[0] == "kill") {
      CHECK_VALID(2, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      flatbuffers::Offset<fbs::KillStep> kill_step = fbs::CreateKillStep(GetFBB(), unit_id);
      copy_to_netbuffer(fbs::StepUnion::KillStep, kill_step.Union());
    }

    else if (tokens[0] == "move") {
      CHECK_VALID(5, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      fbs::v3i dest = (str_to_v3i(tokens[2], tokens[3], tokens[4]));
      bool immediate = (true);
      bool require_ownership = (true);
      flatbuffers::Offset<fbs::MoveStep> move_step = fbs::CreateMoveStep(GetFBB(), unit_id, &dest, s_active_player, immediate, false, false, require_ownership);
      copy_to_netbuffer(fbs::StepUnion::MoveStep, move_step.Union());
    }
    else if (tokens[0] == "pillage") {
      CHECK_VALID(2, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      flatbuffers::Offset<fbs::PillageStep> pillage_step = fbs::CreatePillageStep(GetFBB(), s_active_player, unit_id);
      copy_to_netbuffer(fbs::StepUnion::PillageStep, pillage_step.Union());
    }
    else if (tokens[0] == "queue_move") {
      CHECK_VALID(5, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      fbs::v3i dest = (str_to_v3i(tokens[2], tokens[3], tokens[4]));
      bool immediate = (false);
      bool avoid_unit = false;
      bool avoid_city = false;
      bool require_ownership = (true);
      flatbuffers::Offset<fbs::MoveStep> move_step = fbs::CreateMoveStep(GetFBB(), unit_id, &dest, s_active_player, immediate, avoid_unit, avoid_city, require_ownership);
      copy_to_netbuffer(fbs::StepUnion::MoveStep, move_step.Union());
    }

    else if (tokens[0] == "purchase") {
      CHECK(2, tokens);
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t production_id = 0;
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          production_id = (std::stoul(tokens[2]));
        }
        else {
          production_id = (util::enum_to_uint(get_construction_type(tokens[2])));
        }
      }
      flatbuffers::Offset<fbs::PurchaseStep> purchase_step = fbs::CreatePurchaseStep(GetFBB(), s_active_player, production_id, city_id);
      copy_to_netbuffer(fbs::StepUnion::PurchaseStep, purchase_step.Union());
    }

    else if (tokens[0] == "research") {
      CHECK_VALID(2, tokens);
      uint32_t science_id = (static_cast<uint32_t>(get_science_type(tokens[1])));
      if (science_id == 0) {
        science_id = (std::stoul(tokens[1]));
      }
      flatbuffers::Offset<fbs::ResearchStep> research_step = fbs::CreateResearchStep(GetFBB(), s_active_player, science_id);
      copy_to_netbuffer(fbs::StepUnion::ResearchStep, research_step.Union());
    }
    
    else if (tokens[0] == "sell") {
      CHECK(2, tokens);
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t production_id = 0;
      if (tokens.size() > 2) {
        if (std::isdigit(tokens[2][0])) {
          production_id = (std::stoul(tokens[2]));
        }
        else {
          production_id = (util::enum_to_uint(get_construction_type(tokens[2])));
        }
      }
      flatbuffers::Offset<fbs::SellStep> sell_step = fbs::CreateSellStep(GetFBB(), s_active_player, city_id, production_id);
      copy_to_netbuffer(fbs::StepUnion::SellStep, sell_step.Union());
    }

    else if (tokens[0] == "siege") {
      CHECK_VALID(3, tokens);
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t unit_id = (std::stoul(tokens[2]));
      flatbuffers::Offset<fbs::SiegeStep> siege_step = fbs::CreateSiegeStep(GetFBB(), s_active_player, unit_id, city_id);
      copy_to_netbuffer(fbs::StepUnion::SiegeStep, siege_step.Union());
    }
    else if (tokens[0] == "specialize") {
      CHECK(3, tokens);
      uint32_t city_id = (std::stoul(tokens[1]));
      uint32_t terrain_type = 0;
      if (std::isdigit(tokens[2][0])) {
        terrain_type = (std::stoul(tokens[2]));
      }
      else {
        terrain_type = (util::enum_to_uint(get_terrain_type(tokens[2])));
      }
      flatbuffers::Offset<fbs::SpecializeStep> specialize_step = fbs::CreateSpecializeStep(GetFBB(), city_id, terrain_type, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::SpecializeStep, specialize_step.Union());
    }

    else if (tokens[0] == "spawn") {
      CHECK(5, tokens);
      uint32_t unit_type;
      // If first character in string is a number treat it as an id.
      if (std::isdigit(tokens[1][0])) {
        unit_type = std::stoul(tokens[1]);
      }
      // Else treat it as the name of what needs to be spawned.
      else {
        unit_type = util::enum_to_uint(get_unit_type(tokens[1]));
      }
      fbs::v3i unitLoc = str_to_v3i(tokens[2], tokens[3], tokens[4]);

      flatbuffers::Offset<fbs::SpawnStep> spawn_step  = fbs::CreateSpawnStep(GetFBB(), unit_type, &unitLoc, s_active_player);
      copy_to_netbuffer(fbs::StepUnion::SpawnStep, spawn_step.Union());
    }

    else if (tokens[0] == "stats") {
      CHECK_VALID(5, tokens);
      uint32_t unit_id = (std::stoul(tokens[1]));
      uint32_t health = (std::stoul(tokens[2]));
      uint32_t attack = (std::stoul(tokens[3]));
      uint32_t range = (std::stoul(tokens[4]));
      flatbuffers::Offset<fbs::UnitStatsStep> unit_stats_step = fbs::CreateUnitStatsStep(GetFBB(), unit_id, health, attack, range);
      copy_to_netbuffer(fbs::StepUnion::UnitStatsStep, unit_stats_step.Union());
    }

    else if (tokens[0] == "cast") {
      CHECK(5, tokens);
      fbs::MAGIC_TYPE magic_type = (fbs::MAGIC_TYPE)(get_magic_type(tokens[1]));
      fbs::v3i location = (str_to_v3i(tokens[2], tokens[3], tokens[4]));
      bool cheat = false;
      if (tokens.size() > 5) {
        cheat = true;
      }
      flatbuffers::Offset<fbs::MagicStep> magic_step = fbs::CreateMagicStep(GetFBB(), s_active_player, &location, magic_type, cheat);
      copy_to_netbuffer(fbs::StepUnion::MagicStep, magic_step.Union());
    }

    else if (tokens[0] == "status") {
      CHECK(5, tokens);
      fbs::STATUS_TYPE status_type = (fbs::STATUS_TYPE)(get_status_type(tokens[1]));
      fbs::v3i location = str_to_v3i(tokens[2], tokens[3], tokens[4]);
      flatbuffers::Offset<fbs::StatusStep> status_step = fbs::CreateStatusStep(GetFBB(), status_type, &location);
      copy_to_netbuffer(fbs::StepUnion::StatusStep, status_step.Union());
    }

    else if (tokens[0] == "scenario") {
      CHECK(2, tokens);
      SCENARIO_TYPE scenario_type;
      if (std::isdigit(tokens[1][0])) {
        scenario_type = (util::uint_to_enum<SCENARIO_TYPE>(std::stoul(tokens[1])));
      }
      // Else treat it as a the name of what needs to be spawned.
      else {
        scenario_type = (get_scenario_type(tokens[1]));
      }
      flatbuffers::Offset<fbs::ScenarioStep> scenario_step = fbs::CreateScenarioStep(GetFBB(), (fbs::SCENARIO_TYPE)scenario_type);
      copy_to_netbuffer(fbs::StepUnion::ScenarioStep, scenario_step.Union());
    }

    else {
      std::cout << "Unrecognized step: " << format::vector(tokens) << std::endl;
      return 0;
    }

    if (GetFBB().GetSize() == 0) {
      std::cout << "Failure to serialize step" << std::endl;
      return 0;
    }

    return GetFBB().GetSize();
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

size_t step_parser::parse(const std::vector<std::string>& tokens, void* buffer, size_t buffer_len) {
  return parse_tokens(tokens, buffer, buffer_len);
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

