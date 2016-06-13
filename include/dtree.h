#pragma once

#include <cstdint>

const float NOOP_EVALUATION = 0xffffffff;

// An AI decision is an action an AI takes. Attacking a unit, colonizing, etc.
class Decision {
public:
  virtual ~Decision() {};

  virtual void operator()(uint32_t /*player_id*/) {};
};

// An AI evaluation is used to determine what branch down the decision tree the AI 
// will navigate. A value less than the DNode threshold will recurse left. A value
// greater will recurse right.
class Evaluation {
public:
  virtual ~Evaluation() {};

  virtual float operator()(uint32_t /*player_id*/, float /*threshold*/) { return NOOP_EVALUATION; };
};

struct DNode {
  DNode(Decision* decision, Evaluation* evaluation) :
    m_decision(decision)
    , m_evaluation(evaluation)
    , m_right(nullptr)
    , m_left(nullptr)
    , m_threshold(0.5f) {};
 
  Decision* m_decision;
  Evaluation* m_evaluation;

  DNode* m_right;
  DNode* m_left;
  float m_threshold; 
};

// Decision tree will be an organization of DNodes. They do not control 
// the lifetime of DNodes.
class DTree {
public:
  DTree(DNode* root);

  // Recursively deletes the nodes in this tree.
  void delete_node(DNode* node);
  ~DTree();

  // Run this decision with the given player. 
  void make_decision(uint32_t player_id);

private:
  // Recurse down the tree for a decision.
  void recurse(uint32_t player_id, DNode* node);

  DNode* m_root;
};
