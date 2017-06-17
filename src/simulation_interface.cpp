#include "simulation_interface.h"

#include "step_generated.h"
#include "simulation.h"
#include "random.h"
#include "player.h"
#include "world_map.h"
#include "ai_barbarians.h"

#include <iostream>

namespace simulation_interface {
  flatbuffers::FlatBufferBuilder& GetFBB();
  void copy_to_netbuffer(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step, char* buffer, size_t buffer_len);

  flatbuffers::FlatBufferBuilder& GetFBB() {
    static flatbuffers::FlatBufferBuilder builder;
    return builder;
  }

  void copy_to_netbuffer(fbs::StepUnion step_type, const flatbuffers::Offset<void>& step, char* buffer, size_t buffer_len) {
    flatbuffers::Offset<fbs::AnyStep> anystep = fbs::CreateAnyStep(GetFBB(), step_type, step);
    fbs::FinishAnyStepBuffer(GetFBB(), anystep);
    if (GetFBB().GetSize() > buffer_len) {
      std::cout << "FlatBufferBuilder exceeded network buffer!! StepType: " << (uint32_t)step_type << std::endl;
      return;
    }

    std::memcpy(buffer, GetFBB().GetBufferPointer(), GetFBB().GetSize());
    GetFBB().Clear();
  }

  void start() {
    game_random::set_seed(3);
  }

  void start_faceoff() {
    const size_t BUFFER_LEN = 512;
    char buffer[BUFFER_LEN];
    fbs::SCENARIO_TYPE scenario_type = fbs::SCENARIO_TYPE::FACEOFF;
    flatbuffers::Offset<fbs::ScenarioStep> scenario_step = fbs::CreateScenarioStep(GetFBB(), (fbs::SCENARIO_TYPE)scenario_type);
    copy_to_netbuffer(fbs::StepUnion::ScenarioStep, scenario_step.Union(), buffer, BUFFER_LEN);
    simulation::process_step(buffer, BUFFER_LEN);
  }

  void join_player(fbs::AI_TYPE type, const std::string& name) {
    const size_t BUFFER_LEN = 512;
    char buffer[BUFFER_LEN];
    fbs::AI_TYPE ai_type = type;
    flatbuffers::Offset<flatbuffers::String> fbs_name = GetFBB().CreateString(name);
    flatbuffers::Offset<fbs::AddPlayerStep> add_player_step = fbs::CreateAddPlayerStep(GetFBB(), fbs_name, (fbs::AI_TYPE)ai_type);
    copy_to_netbuffer(fbs::StepUnion::AddPlayerStep, add_player_step.Union(), buffer, BUFFER_LEN);
    simulation::process_step(buffer, BUFFER_LEN);
  }

  void end_turn(uint32_t player_id, uint32_t next_player) {
    const size_t BUFFER_LEN = 512;
    char buffer[BUFFER_LEN];
    flatbuffers::Offset<fbs::EndTurnStep> end_turn_step = fbs::CreateEndTurnStep(GetFBB(), player_id, next_player);
    copy_to_netbuffer(fbs::StepUnion::EndTurnStep, end_turn_step.Union(), buffer, BUFFER_LEN);
    simulation::process_step(buffer, BUFFER_LEN);
  }
}

extern "C" {
  void simulation_reset() {
    simulation::reset();
  }

  void simulation_start() {
    simulation_interface::start();
  }

  void simulation_end() {
    simulation::kill();
  }

  void simulation_start_faceoff() {
    simulation_interface::start_faceoff();
  }

  void simulation_join_barbarian(const char* name) {
    simulation_interface::join_player(fbs::AI_TYPE::BARBARIAN, name);
  }

  void simulation_join_player(const char* name) {
    simulation_interface::join_player(fbs::AI_TYPE::HUMAN, name);
  }

  int simulation_players_size() {
    int count = 0;
    player::for_each_player([&count](Player& player) {
      ++count;
    });
    return count;
  }

  Unit* simulation_units_create() {
    Unit* units = new Unit[unit::size()];
    return units;
  }

  void simulation_units_sync(Unit* units) {
    int i = 0;
    unit::for_each_unit([&units, &i](const Unit& unit) {
      units[i++] = unit;
    });
  }

  void simulation_units_free(Unit* units) {
    delete[] units;
  }

  int simulation_units_size() {
    return unit::size();
  }

  int simulation_units_x(Unit* units, int i) {
    return units[i].m_location.x;
  }

  int simulation_units_y(Unit* units, int i) {
    return units[i].m_location.y;
  }

  int simulation_units_z(Unit* units, int i) {
    return units[i].m_location.z;
  }

  unsigned int simulation_units_id(Unit* units, int i) {
    return units[i].m_id;
  }

  unsigned int simulation_units_owner_id(Unit* units, int i) {
    return units[i].m_owner_id;
  }

  Tile* simulation_tiles_create() {
    world_map::TileMap& map = world_map::get_map();
    Tile* tiles = new Tile[map.size()];
    return tiles;
  }

  void simulation_tiles_sync(Tile* tiles) {
    world_map::TileMap& map = world_map::get_map();
    int i = 0;
    for (const auto& tile : map) {
      tiles[i++] = tile.second;
    }
  }

  void simulation_tiles_free(Tile* tiles) {
    delete[] tiles;
  }

  int simulation_tiles_size() {
    return world_map::get_map().size();
  }

  int simulation_tiles_x(Tile* tiles, int i) {
    return tiles[i].m_location.x;
  }

  int simulation_tiles_y(Tile* tiles, int i) {
    return tiles[i].m_location.y;
  }

  int simulation_tiles_z(Tile* tiles, int i) {
    return tiles[i].m_location.z;
  }

  void simulation_end_turn(int current_player, int next_player) {
    simulation_interface::end_turn(current_player, next_player);
  }

  void simulation_barbarians_set_id(int player_id) {
    barbarians::set_player_id(player_id);
  }

  void simulation_barbarians_execute_turn(int player_id) {
    barbarians::pillage_and_plunder(player_id);
  }

  void simulation_barbarians_reset() {
    barbarians::reset();
  }
}
