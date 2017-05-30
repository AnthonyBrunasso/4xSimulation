#pragma once

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "tile.h"

namespace simulation_interface {
  void start();
  void start_faceoff();
  void join_player(fbs::AI_TYPE type, const std::string& name);
  void end_turn(uint32_t player_id, uint32_t next_player);
}

extern "C" {
  // Simulation steps (mostly)
  __declspec(dllexport) void simulation_start();
  __declspec(dllexport) void simulation_end();
  __declspec(dllexport) void simulation_start_faceoff();
  __declspec(dllexport) void simulation_join_barbarian(const char* name);
  __declspec(dllexport) void simulation_join_player(const char* name);
  __declspec(dllexport) void simulation_end_turn(int current_player, int next_player);

  // Player
  __declspec(dllexport) int simulation_players_size();

  // Tile Map
  __declspec(dllexport) Tile* simulations_tiles_create();
  __declspec(dllexport) void simulation_tiles_sync(Tile* tiles);
  __declspec(dllexport) void simulation_tiles_free(Tile* tiles);
  __declspec(dllexport) int simulation_tiles_size(Tile* tiles);
  __declspec(dllexport) int simulation_tiles_x(Tile* tiles, int i);
  __declspec(dllexport) int simulation_tiles_y(Tile* tiles, int i);
  __declspec(dllexport) int simulation_tiles_z(Tile* tiles, int i);

  // Barbarians
  __declspec(dllexport) void simulation_barbarians_set_id(int player_id);
  __declspec(dllexport) void simulation_barbarians_execute_turn(int player_id);
  __declspec(dllexport) void simulation_barbarians_reset();
}