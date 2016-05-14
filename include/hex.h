#pragma once

#include "Vector2.hpp"
#include "Vector3.hpp"

#include <cstdint>
#include <vector>

namespace hex
{
  // Hexagons coordinates have 6 neighbors
  const static unsigned int NEIGHBOR_COUNT = 6;

  // Given a hexagon, the directos are:
  //        
  //  North West   / \   North East
  //  West        |   |  East
  //  South West   \ /   South East
  // 

  // Cube coordinates for hex grid must maintain equality: x + y + z = 0
  const static sf::Vector3i CUBE_DIRECTIONS[NEIGHBOR_COUNT] = {
    sf::Vector3i(1, -1, 0), // North East
    sf::Vector3i(1, 0, -1), // East
    sf::Vector3i(0, 1, -1), // South East
    sf::Vector3i(-1, 1, 0), // South West
    sf::Vector3i(-1, 0, 1), // West
    sf::Vector3i(0, -1, 1), // North West
  };

  // Axial coordinates simply drop the third coordinate of cube
  const static sf::Vector2i AXIAL_DIRECTIONS[NEIGHBOR_COUNT] = {
    sf::Vector2i(1 ,  0), // East
    sf::Vector2i(1 , -1), // North East
    sf::Vector2i(0 , -1), // North West
    sf::Vector2i(-1,  0), // West
    sf::Vector2i(-1,  1), // South West
    sf::Vector2i(0 ,  1), // South East
  };

  struct CubeNeighbors {
    CubeNeighbors(const sf::Vector3i& start);
    sf::Vector3i& operator[](uint32_t i) { return neighbors[i]; }
    sf::Vector3i neighbors[NEIGHBOR_COUNT];
  };

  struct AxialNeighbors {
    AxialNeighbors(const sf::Vector2i& start);
    sf::Vector2i& operator[](uint32_t i) { return neighbors[i]; }
    sf::Vector2i neighbors[NEIGHBOR_COUNT];
  };

  // Grid conversions
  sf::Vector2i cube_to_axial(const sf::Vector3i& cube_coord);
  sf::Vector2f cube_to_axial(const sf::Vector3f& cube_coord);

  sf::Vector2i cube_to_offset(const sf::Vector3i& cube_coord);
  sf::Vector2i axial_to_offset(const sf::Vector2i& axial_coord);

  sf::Vector3i axial_to_cube(const sf::Vector2i& axial_coord);
  sf::Vector3f axial_to_cube(const sf::Vector2f& axial_coord);

  // Get the pixel coordinates to the i'th corner of a hexagon offset from (0,0)
  sf::Vector2f hex_corner(uint32_t size, uint32_t i);

  // Hex and pixel conversions
  sf::Vector2f axial_to_world(const sf::Vector2i& axial_coord, uint32_t size);
  sf::Vector2f cube_to_world(const sf::Vector3i& cube_coord, uint32_t size);
  sf::Vector2f offset_to_world(const sf::Vector2i& axial_coord, uint32_t size);

  sf::Vector2i world_to_axial(const sf::Vector2f& world_coord, uint32_t size);
  sf::Vector3i world_to_cube(const sf::Vector2f& world_coord, uint32_t size);
  sf::Vector2i world_to_offset(const sf::Vector2f& world_coord, uint32_t size);

  // Rounding pixel coord to axial and cube coordinates
  // Cube coordinates must mantain x + y + z = 0 therefore a round of componenets isn't enough
  sf::Vector3i cube_round(const sf::Vector3f& pixel_coord);
  sf::Vector2i axial_round(const sf::Vector2f& pixel_coord);

  // Distance of coordinates
  uint32_t cube_distance(const sf::Vector3i& a, const sf::Vector3i& b);
  uint32_t axial_distance(const sf::Vector2i& a, const sf::Vector2i& b);

  // This function requires floating point cube-coordinates else a lerp on integer
  // vectors would likely not maintain the equality x + y + z = 0
  void cubes_on_line(const sf::Vector3f& a, const sf::Vector3f& b, std::vector<sf::Vector3i>& coords);
}