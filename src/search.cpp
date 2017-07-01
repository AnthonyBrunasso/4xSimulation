#include "search.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "city.h"
#include "enum_generated.h"
#include "hex.h"
#include "improvement.h"
#include "resources.h"
#include "tile.h"
#include "unit.h"
#include "world_map.h"

namespace {
  struct PathNode {
    PathNode(const sf::Vector3i& location, uint32_t cost, uint32_t heuristic) 
      : m_location(location)
      , m_cost(cost)
      , m_heuristic(heuristic) {};

    sf::Vector3i m_location;
    uint32_t m_cost;
    uint32_t m_heuristic;

  private:
    PathNode();
  };

  struct PathNodeComparator {
    bool operator() (const PathNode& lhs, const PathNode& rhs) { 
      return lhs.m_heuristic > rhs.m_heuristic; 
    }
  };

  // Helper function to get tile cost with a default value 
  uint32_t get(const std::unordered_map<sf::Vector3i, uint32_t> &costs, const sf::Vector3i& tile) {
    auto it = costs.find(tile);
    if (it == costs.end()) {
      return UINT32_MAX;
    }

    return it->second;
  }

  // Cube distance will work as a well behaved heuristic for A* pathing.
  uint32_t heuristic_estimate(const sf::Vector3i& from, const sf::Vector3i& goal) {
    return hex::cube_distance(from, goal);
  }

  void build_path(const sf::Vector3i& target, 
      std::unordered_map<sf::Vector3i, sf::Vector3i>& came_from, 
      std::vector<sf::Vector3i>& path) {
    sf::Vector3i current = target;
    path.push_back(current);
    while (came_from.find(current) != came_from.end()) {
      current = came_from[current];
      path.push_back(current);
    }
    // Path in reverse order so reverse it.
    std::reverse(path.begin(), path.end());
  } 
}

std::vector<sf::Vector3i> search::range(const sf::Vector3i& start, int32_t distance) {
  std::vector<sf::Vector3i> coords;
  for (int32_t dx = -distance; dx <= distance; ++dx) {
    int32_t s = std::max(-distance, -dx - distance);
    for (int32_t dy = s; dy <= std::min(distance, -dx + distance); ++dy) {
      int32_t dz = -dx - dy;
      coords.push_back(start + sf::Vector3i(dx, dy, dz));
    }
  }

  return (coords);
}

// Calculates the path with the cheapest cumulative tile path cost
std::vector<sf::Vector3i> search::path_to(const sf::Vector3i& start,
    const sf::Vector3i& end,
    std::function<bool(const Tile& tile)> expand) {
  std::vector<sf::Vector3i> coords;
  // All the discovered nodes that require evaluation.
  std::priority_queue<PathNode, std::vector<PathNode>, PathNodeComparator> open;
  open.push(PathNode(start, 0, heuristic_estimate(start, end))); 
  // Set of open list discoveries for quick lookup. Unordered map because set uses tree and needs >,< operator.
  std::unordered_map<sf::Vector3i, uint32_t> openDiscovered; 
  openDiscovered[start] = 1;
  // All nodes that are already evaluated. Unordered map because set uses tree and needs >,< operator.
  std::unordered_map<sf::Vector3i, uint32_t> closed;

  // Map used to move backwards from goal node to start to get pstartath.
  std::unordered_map<sf::Vector3i, sf::Vector3i> came_from;
  // The actual costs from the start node to a given node.
  std::unordered_map<sf::Vector3i, uint32_t> true_costs;
  // Cost from start to start is 0.
  true_costs[start] = 0;

  while (!open.empty()) {
    // Back will return path node with least path cost.
    PathNode current = open.top();
    if (current.m_location == end) {
      build_path(current.m_location, came_from, coords);
      return (coords);
    }
    // Remove from open list.
    open.pop();
    openDiscovered.erase(current.m_location);

    // Put into closed list.
    closed[current.m_location] = 1;

    // Get all of currents neighbors.
    std::vector<sf::Vector3i> cube_neighbors;
    hex::cube_neighbors(current.m_location, cube_neighbors); 
    // Loop over neighbors and evaluate state of each node in path.
    for (auto neighbor : cube_neighbors) {
      // Ignore neighbors that have already been evaluated.
      if (closed.find(neighbor) != closed.end()) continue;
      // If the neighbors location equals end, this is our destination, don't skip it or A* will never finish.
      // If the node shouldn't be expanded add it to the closed list and continue.
      if (neighbor != end && expand && !expand(*world_map::get_tile(neighbor))) {
        closed[neighbor] = 1;
        continue;
      }
      PathNode pn(neighbor,
            current.m_cost + world_map::get_tile(neighbor)->m_path_cost,                                    // Cost of current record to this node
            current.m_cost + world_map::get_tile(neighbor)->m_path_cost + heuristic_estimate(neighbor, end)); // Heuristic cost
      // If not in open list, add it for evaluation.
      if (openDiscovered.find(neighbor) == openDiscovered.end()) {
        open.push(pn);
        openDiscovered[neighbor] = 1;
      }
      // If this is not a better path than one already found continue.
      else if (pn.m_cost > get(true_costs, neighbor)) {
        continue;
      }
      came_from[neighbor] = current.m_location;
      true_costs[neighbor] = pn.m_cost;
    }
  }

  return (coords);
}

// Bfs until the comparator returns true.
bool search::bfs(const sf::Vector3i& start,
    uint32_t depth,
    std::function<bool(const Tile& tile)> comparator) {
  std::queue<sf::Vector3i> to_explore;
  // Used to avoid expanding nodes already explored.
  std::unordered_map<sf::Vector3i, uint32_t> discovered;
  to_explore.push(start);
  // Start node is at depth 1.
  discovered[start] = 1;
  while (!to_explore.empty()) {
    sf::Vector3i expand = to_explore.front();
    std::vector<sf::Vector3i> cube_neighbors;
    hex::cube_neighbors(expand, cube_neighbors); 
    if (comparator(*world_map::get_tile(expand))) return true;
    to_explore.pop();
    for (auto n : cube_neighbors) {
      if (discovered.find(n) != discovered.end()) continue;
      if (!world_map::get_tile(n)) continue;
      // Early out if the condition is meet.
      uint32_t current_depth = discovered[expand];
      if (current_depth <= depth) {
        to_explore.push(n);
        discovered[n] = current_depth + 1;
      }
    }
  }
  return false;
}

bool search::bfs_units(const sf::Vector3i& start,
    uint32_t depth,
    std::function<bool(const Unit& unit)> comparator)
{
  auto find_units = [&comparator](const Tile& tile) {
    if (tile.m_unit_ids.empty()) return false;
    bool result = false;
    for (auto id : tile.m_unit_ids) {
      Unit* unit = unit::get_unit(id);
      if (!unit) continue;
      result |= comparator(*unit); 
    }
    return result;
  };
  return bfs(start, depth, find_units);
}

bool search::bfs_cities(const sf::Vector3i& start,
    uint32_t depth,
    std::function<bool(const City& unit)> comparator) {
  auto find_cities = [&comparator](const Tile& tile) {
    if (!tile.m_city_id) return false;
    City* c = city::get_city(tile.m_city_id);
    if (!c) return false;
    return comparator(*c);
  };
  return bfs(start, depth, find_cities);
}

// Run bfs for each improvement to depth.
bool search::bfs_improvements(const sf::Vector3i& start,
    uint32_t depth,
    std::function<bool(const Improvement& unit)> comparator) {
  auto find_improvements = [&comparator](const Tile& tile) {
    if (tile.m_improvement_ids.empty()) return false;
    bool result = false;
    for (auto id : tile.m_improvement_ids) {
      Improvement* i = improvement::get_improvement(id);
      if (!i) continue;
      result |= comparator(*i); 
    }
    return result;
  };
  return bfs(start, depth, find_improvements);
}

// Run bfs for each resource to depth.
bool search::bfs_resources(const sf::Vector3i& start,
    uint32_t depth,
    std::function<bool(const Resource& unit)> comparator) {
  auto find_resources = [&comparator](const Tile& tile) {
    if (tile.m_resource.m_type == fbs::RESOURCE_TYPE::UNKNOWN) return false;
    bool result = false;
    result |= comparator(tile.m_resource);
    return result;
  };
  return bfs(start, depth, find_resources);
}


