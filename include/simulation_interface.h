#pragma once

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "game_types.h"
#include "tile.h"

namespace simulation_interface {
  void start();
  void start_faceoff();
  void join_player(AI_TYPE type, const std::string& name);
  void end_turn(uint32_t player_id, uint32_t next_player);
}

extern "C" {
  // Simulation
  __declspec(dllexport) void simulation_start();
  __declspec(dllexport) void simulation_start_faceoff();
  __declspec(dllexport) void simulation_join_barbarian(const char* name);
  __declspec(dllexport) void simulation_join_player(const char* name);
  __declspec(dllexport) int simulation_count_players();

  // Tile Map
  __declspec(dllexport) Tile* simulation_create_tiles();
  __declspec(dllexport) void simulation_sync_tiles(Tile* tiles);
  __declspec(dllexport) void simulation_free_tiles(Tile* tiles);
  __declspec(dllexport) int simulation_tiles_size(Tile* tiles);
  __declspec(dllexport) int simulation_tile_x(Tile* tiles, int i);
  __declspec(dllexport) int simulation_tile_y(Tile* tiles, int i);
  __declspec(dllexport) int simulation_tile_z(Tile* tiles, int i);
}