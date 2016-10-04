#include "dtree.h"
#include "ai_state.h"
#include <iostream>

void track_node(DNode* node, std::vector<DNode*>& node_vector) {
  if (!node) return;
  track_node(node->m_right, node_vector);
  track_node(node->m_left, node_vector);
  node_vector.push_back(node);
}

DTree::DTree(DNode* root) :
   m_root(root) {
}

DTree::~DTree() {
}

void DTree::track_nodes(std::vector<DNode*>& nodes) {
  track_node(m_root, nodes);
}

// Run this decision with the given player. 
void DTree::make_decision(uint32_t id) {
  if (!m_root) {
    return;
  }

  recurse(id, m_root);
}

void DTree::recurse(uint32_t id, DNode* node) {
  if (!node) return;

  Decision* decision = node->m_decision;
  Evaluation* evaluation = node->m_evaluation;
  float threshold = node->m_threshold;
  
  if (decision) {
    decision->m_state = m_state;
    (*decision)(id);
  }

  float result = NOOP_EVALUATION;
  if (evaluation) {
    evaluation->m_state = m_state;
    result = (*evaluation)(id, threshold);

    if (result == NOOP_EVALUATION) {
      std::cout << "Eval ended in NOOP" << std::endl;
    }
  }

  // If no evaluation was made stop recursing here, the AI is either
  // done or has no idea what do next.
  if (result == NOOP_EVALUATION) {
    return;
  }

  // Recurse right if the result was greater than the threshold.
  if (result > threshold && node->m_right) {
    recurse(id, node->m_right);
  }
  // Recurse left otherwise.
  else if (node->m_left) {
    recurse(id, node->m_left);
  }
}
