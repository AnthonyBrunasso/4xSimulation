#include "hex.h"

#include "custom_math.h"

#include <algorithm> 
#include <cstdlib>
#include <cmath>

namespace {
  const static float PI = 3.1415926535897f;
  const static float SQRT_3 = 1.73205080757f;
}

hex::CubeNeighbors::CubeNeighbors(const sf::Vector3i& start) {
  for (uint32_t i = 0; i < NEIGHBOR_COUNT; ++i) {
    neighbors[i] = start + CUBE_DIRECTIONS[i];
  }
}

hex::AxialNeighbors::AxialNeighbors(const sf::Vector2i& start) {
  for (uint32_t i = 0; i < NEIGHBOR_COUNT; ++i) {
    neighbors[i] = start + AXIAL_DIRECTIONS[i];
  }
}

sf::Vector2i hex::cube_to_axial(const sf::Vector3i& cube_coord) {
  return sf::Vector2i(cube_coord.x, cube_coord.y);
}

sf::Vector2f hex::cube_to_axial(const sf::Vector3f& cube_coord) {
  return sf::Vector2f(cube_coord.x, cube_coord.y);
}

sf::Vector2i hex::cube_to_offset(const sf::Vector3i& cube_coord) {
  int32_t col = cube_coord.x + (cube_coord.z - (cube_coord.z & 1)) / 2;
  int32_t row = cube_coord.z;
  return sf::Vector2i(col, row);
}

sf::Vector2i hex::axial_to_offset(const sf::Vector2i& axial_coord) {
  return cube_to_offset(axial_to_cube(axial_coord));
}

sf::Vector3i hex::axial_to_cube(const sf::Vector2i& axial_coord) {
  return sf::Vector3i(axial_coord.x, axial_coord.y, -axial_coord.x - axial_coord.y);
}

sf::Vector3f hex::axial_to_cube(const sf::Vector2f& axial_coord) {
  return sf::Vector3f(axial_coord.x, axial_coord.y, -axial_coord.x - axial_coord.y);
}

sf::Vector2f hex::hex_corner(uint32_t size, uint32_t i) {
  // Corners of hex are 60 degrees apart starting at 30 degress (lower right corner)
  const uint32_t angle = 60 * i + 30;
  const float rad = PI / 180.0f * angle;
  return sf::Vector2f(size * cos(rad), size * sin(rad));
}

sf::Vector2f hex::axial_to_world(const sf::Vector2i& axial_coord, uint32_t size) {
  const float x = size * SQRT_3 * (axial_coord.x + (axial_coord.y / 2.0f));
  const float y = size * (3.0f / 2.0f) * axial_coord.y;
  return sf::Vector2f(x, y);
}

sf::Vector2f hex::cube_to_world(const sf::Vector3i& cube_coord, uint32_t size) {
  return axial_to_world(cube_to_axial(cube_coord), size);
}

sf::Vector2f hex::offset_to_world(const sf::Vector2i& axial_coord, uint32_t size) {
  // Bitwise & is to tell if the cord is odd or not. We offset every odd row
  const float x = size * SQRT_3 * (axial_coord.x + 0.5f * (axial_coord.y & 1));
  const float y = size * (3.0f / 2.0f) * axial_coord.y;
  return sf::Vector2f(x, y);
}

sf::Vector2i hex::world_to_axial(const sf::Vector2f& world_coord, uint32_t size) {
  const float x = (world_coord.x * (SQRT_3 / 3.0f) - world_coord.y / 3.0f) / size;
  const float y = world_coord.y * (2.0f / 3.0f) / size;
  return axial_round(sf::Vector2f(x, y));
}

sf::Vector3i hex::world_to_cube(const sf::Vector2f& world_coord, uint32_t size) {
  return axial_to_cube(world_to_axial(world_coord, size));
}

sf::Vector2i hex::world_to_offset(const sf::Vector2f& world_coord, uint32_t size) {
  return axial_to_offset(world_to_axial(world_coord, size));
}

// Rounding must maintain x + y + z = 0 
sf::Vector3i hex::cube_round(const sf::Vector3f& cube_coord) {
  // Round all the components
  int32_t rx = static_cast<int32_t>(roundf(cube_coord.x));
  int32_t ry = static_cast<int32_t>(roundf(cube_coord.y));
  int32_t rz = static_cast<int32_t>(roundf(cube_coord.z));

  // Fix them to maintain equality
  const float dx = fabs(rx - cube_coord.x);
  const float dy = fabs(ry - cube_coord.y);
  const float dz = fabs(rz - cube_coord.z);

  if (dx > dy && dx > dz) {
    rx = -ry - rz;
  }
  else if (dy > dz) {
    ry = -rx - rz;
  }
  else {
    rz = -rx - ry;
  }

  return sf::Vector3i(rx, ry, rz);
}

sf::Vector2i hex::axial_round(const sf::Vector2f& axial_coord) {
  return cube_to_axial(cube_round(axial_to_cube(axial_coord)));
}

uint32_t hex::cube_distance(const sf::Vector3i& a, const sf::Vector3i& b) {
  return std::max( { std::abs(a.x - b.x), std::abs(a.y - b.y), std::abs(a.z - b.z) } );
}

uint32_t hex::axial_distance(const sf::Vector2i& a, const sf::Vector2i& b) {
  // Convert to cube coordinates and get distance
  sf::Vector3i ac = axial_to_cube(a);
  sf::Vector3i bc = axial_to_cube(b);
  return cube_distance(ac, bc);
}

void hex::cubes_on_line(const sf::Vector3f& a, const sf::Vector3f& b, std::vector<sf::Vector3i>& coords) {
  uint32_t distance = cube_distance(cube_round(a), cube_round(b));
  for (uint32_t i = 0; i <= distance; ++i) {
    coords.push_back(cube_round(cmath::lerp(a, b, 1.0f / distance * i)));
  }
}