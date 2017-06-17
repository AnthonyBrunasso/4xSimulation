#pragma once

#include "flatbuffers/flatbuffers.h"
#include "step_generated.h"
#include "tile.h"
#include "unit.h"

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#elif defined(__clang__)
#define DLL_EXPORT __attribute__((__visibility__("default")))
#endif


namespace simulation_interface {
  void start();
  void start_faceoff();
  void join_player(fbs::AI_TYPE type, const std::string& name);
  void end_turn(uint32_t player_id, uint32_t next_player);
}

extern "C" {
  // Simulation steps (mostly)
  DLL_EXPORT void simulation_reset();
  DLL_EXPORT void simulation_start();
  DLL_EXPORT void simulation_end();
  DLL_EXPORT void simulation_start_faceoff();
  DLL_EXPORT void simulation_join_barbarian(const char* name);
  DLL_EXPORT void simulation_join_player(const char* name);
  DLL_EXPORT void simulation_end_turn(int current_player, int next_player);

  // Player
  DLL_EXPORT int simulation_players_size();

  // Unit
  DLL_EXPORT Unit* simulation_units_create();
  DLL_EXPORT void simulation_units_sync(Unit* units);
  DLL_EXPORT void simulation_units_free(Unit* units);
  DLL_EXPORT int simulation_units_size();
  DLL_EXPORT int simulation_units_x(Unit* units, int i);
  DLL_EXPORT int simulation_units_y(Unit* units, int i);
  DLL_EXPORT int simulation_units_z(Unit* units, int i);
  DLL_EXPORT unsigned int simulation_units_id(Unit* units, int i);
  DLL_EXPORT unsigned int simulation_units_owner_id(Unit* units, int i);

  // Tile Map
  DLL_EXPORT Tile* simulation_tiles_create();
  DLL_EXPORT void simulation_tiles_sync(Tile* tiles);
  DLL_EXPORT void simulation_tiles_free(Tile* tiles);
  DLL_EXPORT int simulation_tiles_size();
  DLL_EXPORT int simulation_tiles_x(Tile* tiles, int i);
  DLL_EXPORT int simulation_tiles_y(Tile* tiles, int i);
  DLL_EXPORT int simulation_tiles_z(Tile* tiles, int i);

  // Barbarians
  DLL_EXPORT void simulation_barbarians_set_id(int player_id);
  DLL_EXPORT void simulation_barbarians_execute_turn(int player_id);
  DLL_EXPORT void simulation_barbarians_reset();
}
