#pragma once

#include <cstdint>
#include <cfloat>
#include <vector>

struct TurnState;
const float NOOP_EVALUATION = FLT_MAX;

// An AI decision is an action an AI takes. Attacking a unit, colonizing, etc.
class Decision {
public:
  virtual ~Decision() {};

  virtual void operator()(uint32_t /*id*/) {};
  TurnState* m_state;
};

// An AI evaluation is used to determine what branch down the decision tree the AI 
// will navigate. A value less than the DNode threshold will recurse left. A value
// greater will recurse right.
class Evaluation {
public:
  virtual ~Evaluation() {};

  virtual float operator()(uint32_t /*id*/, float /*threshold*/) { return NOOP_EVALUATION; };
  TurnState* m_state;
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
  ~DTree();

  // Run this decision with the given player.
  void make_decision(uint32_t id);

  TurnState* m_state;
private:
  // Recurse down the tree for a decision.
  void recurse(uint32_t id, DNode* node);

  DNode* m_root;
};
