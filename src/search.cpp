#include "search.h"

#include "hex.h"
#include "tile.h"
#include "format.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <unordered_map>

namespace {
  // Score struct used in A* pathfinding cost maps.
  // Default value infinity because that's the initial distance known from start to 
  // a random node given we have not evaluated a valid path to the node.
  struct Score {
    Score() : m_value(UINT32_MAX) {};
    Score(uint32_t value) : m_value(value) {};

    uint32_t m_value;
  };

  struct PathNode {
    PathNode(const sf::Vector3i& location, uint32_t cost, uint32_t heuristic) :
      m_location(location)
      , m_cost(cost)
      , m_heuristic(heuristic) {};

    sf::Vector3i m_location;
    Score m_cost;
    Score m_heuristic;
  };

  struct PathNodeComparator {
    bool operator() (const PathNode& lhs, const PathNode& rhs) { 
      return lhs.m_heuristic.m_value > rhs.m_heuristic.m_value; 
    }
  };
 
  /*auto s_comparator = [](const PathNode& lhs, const PathNode& rhs) { 
    return lhs.m_heuristic.m_value > rhs.m_heuristic.m_value; 
  };*/

  // Helper function to set the actual value of a score.
  void set(Score& score, uint32_t value) {
    score.m_value = value;
  }

  // Cube distance will work as a well behaved heuristic for A* pathing.
  uint32_t heuristic_estimate(const sf::Vector3i& from, const sf::Vector3i& goal) {
    return hex::cube_distance(from, goal);
  }

  // Appends the list and formats it as a heap.
/*  void push_heap(std::vector<PathNode>& list, const PathNode& record) {
    list.push_back(record);
    std::push_heap(list.begin(), list.end(), s_comparator);
  }

  // Pops the list and maintains heap structure.
  void pop_heap(std::vector<PathNode>& list) {
    std::pop_heap(list.begin(), list.end(), s_comparator);
    list.pop_back();
  }*/

  void find_neighbors(const PathNode& record, const sf::Vector3i& goal, std::vector<PathNode>& neighbors, world_map::TileMap& tile_map) {
    // This is a little inefficient since this will need to be copied into neighbors.
    std::vector<sf::Vector3i> cube_neighbors;
    hex::cube_neighbors(record.m_location, cube_neighbors); 

    // Fill neighbors with path nodes
    neighbors.clear();
    for (auto node : cube_neighbors) {
      // If the node does not exist in the map continue.
      if (tile_map.find(node) == tile_map.end()) continue;

      neighbors.push_back(
          PathNode(node, 
            record.m_cost.m_value + tile_map[node].m_path_cost,                                    // Cost of current record to this node
            record.m_cost.m_value + tile_map[node].m_path_cost + heuristic_estimate(node, goal))); // Heuristic cost

      std::cout << "found neighbors: " << format::vector3(node) << " cost: " << neighbors.back().m_cost.m_value << " heuristic: " << neighbors.back().m_heuristic.m_value << std::endl;
    }
  }

  void build_path(const sf::Vector3i& target, std::unordered_map<sf::Vector3i, sf::Vector3i>& came_from, std::vector<sf::Vector3i>& path) {
    sf::Vector3i current = target;
    path.push_back(current);
    while (came_from.find(current) != came_from.end()) {
      current = came_from[current];
      path.push_back(current);
    }
    std::cout << "build_path: " << path.size() << std::endl;
    // Path in reverse order so reverse it.
    std::reverse(path.begin(), path.end());
  } 
}

void search::range(const sf::Vector3i& start, int32_t distance, std::vector<sf::Vector3i>& coords) {
  for (int32_t dx = -distance; dx <= distance; ++dx) {
    int32_t s = std::max(-distance, -dx - distance);
    for (int32_t dy = s; dy <= std::min(distance, -dx + distance); ++dy) {
      int32_t dz = -dx - dy;
      coords.push_back(start + sf::Vector3i(dx, dy, dz));
    }
  }
}

// Calculates the path with the cheapest cumulative tile path cost
void search::path_to(const sf::Vector3i& start,
    const sf::Vector3i& end,
    world_map::TileMap& tile_map, 
    std::vector<sf::Vector3i>& coords) {
  // All the discovered nodes that require evaluation.
//  std::vector<PathNode> open { PathNode(start, 0, heuristic_estimate(start, end)) }; 
  std::priority_queue<PathNode, std::vector<PathNode>, PathNodeComparator> open;
  open.push(PathNode(start, 0, heuristic_estimate(start, end))); 
  // Set of open list discoveries for quick lookup. Unordered map because set uses tree and needs >,< operator.
  std::unordered_map<sf::Vector3i, bool> openDiscovered; 
  openDiscovered[start] = true;
  // All nodes that are already evaluated. Unordered map because set uses tree and needs >,< operator.
  std::unordered_map<sf::Vector3i, bool> closed;

  // Map used to move backwards from goal node to start to get path.
  std::unordered_map<sf::Vector3i, sf::Vector3i> came_from;
  // The actual costs from the start node to a given node.
  std::unordered_map<sf::Vector3i, Score> true_costs;
  // Cost from start to start is 0.
  set(true_costs[start], 0);

  while (!open.empty()) {
    // Back will return path node with least path cost.
    PathNode current = open.top();
    std::cout << "min: " << format::vector3(current.m_location) << std::endl;
    if (current.m_location == end) {
      std::cout << "Found end!" << std::endl;
      build_path(current.m_location, came_from, coords);
      return;
    }
    // Remove from open list.
    open.pop();
    openDiscovered.erase(current.m_location);

    // Put into closed list.
    closed[current.m_location] = true;

    // Get all of currents neighbors.
    std::vector<PathNode> neighbors;
    find_neighbors(current, end, neighbors, tile_map);

    // Loop over neighbors and evaluate state of each node in path.
    for (auto neighbor : neighbors) {
      // Ignore neighbors that have already been evaluated.
      if (closed.find(neighbor.m_location) != closed.end()) continue;
      // If not in open list, add it for evaluation.
      if (openDiscovered.find(neighbor.m_location) == openDiscovered.end()) {
        open.push(neighbor);
        openDiscovered[neighbor.m_location] = true;
      }
      // If this is not a better path than one already found continue.
      else if (neighbor.m_cost.m_value > true_costs[neighbor.m_location].m_value) {
        continue;
      }
      came_from[neighbor.m_location] = current.m_location;
      set(true_costs[neighbor.m_location], neighbor.m_cost.m_value);
    }
  }
}

