#include "simulation_interface.h"

#include "step_generated.h"
#include "simulation.h"
#include "random.h"
#include "player.h"
#include "world_map.h"

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
    simulation::start();
  }

  void start_faceoff() {
    const size_t BUFFER_LEN = 512;
    char buffer[BUFFER_LEN];
    SCENARIO_TYPE scenario_type = SCENARIO_TYPE::FACEOFF;
    flatbuffers::Offset<fbs::ScenarioStep> scenario_step = fbs::CreateScenarioStep(GetFBB(), (fbs::SCENARIO_TYPE)scenario_type);
    copy_to_netbuffer(fbs::StepUnion::ScenarioStep, scenario_step.Union(), buffer, BUFFER_LEN);
    simulation::process_step(buffer, BUFFER_LEN);
  }

  void join_player(AI_TYPE type, const std::string& name) {
    const size_t BUFFER_LEN = 512;
    char buffer[BUFFER_LEN];
    AI_TYPE ai_type = type;
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
  void simulation_start() {
    simulation_interface::start();
  }

  void simulation_start_faceoff() {
    simulation_interface::start_faceoff();
  }

  void simulation_join_barbarian(const char* name) {
    simulation_interface::join_player(AI_TYPE::BARBARIAN, name);
  }

  void simulation_join_player(const char* name) {
    simulation_interface::join_player(AI_TYPE::HUMAN, name);
  }

  int simulation_count_players() {
    int count = 0;
    player::for_each_player([&count](Player& player) {
      ++count;
    });
    return count;
  }

  Tile* simulation_create_tiles() {
    world_map::TileMap& map = world_map::get_map();
    Tile* tiles = new Tile[map.size()];
    return tiles;
  }

  void simulation_sync_tiles(Tile* tiles) {
    world_map::TileMap& map = world_map::get_map();
    int i = 0;
    for (const auto& tile : map) {
      tiles[i++] = tile.second;
    }
  }

  void simulation_free_tiles(Tile* tiles) {
    delete[] tiles;
  }

  int simulation_tiles_size(Tile* tiles) {
    return world_map::get_map().size();
  }

  int simulation_tile_x(Tile* tiles, int i) {
    return tiles[i].m_location.x;
  }

  int simulation_tile_y(Tile* tiles, int i) {
    return tiles[i].m_location.y;
  }

  int simulation_tile_z(Tile* tiles, int i) {
    return tiles[i].m_location.z;
  }
}