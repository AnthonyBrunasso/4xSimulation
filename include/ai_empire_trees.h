#pragma once

class DTree;
struct TurnState;

namespace empire_trees {
  DTree& get_primitive_macro();
  DTree& get_primitive_micro(TurnState*);

  void shutdown();
}