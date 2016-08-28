#include "dtree.h"

DTree::DTree(DNode* root) :
   m_root(root) {
}

DTree::~DTree() {
  delete_node(m_root);
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
    (*decision)(id);
  }

  float result = NOOP_EVALUATION;
  if (evaluation) {
    result = (*evaluation)(id, threshold);
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

void DTree::delete_node(DNode* node) {
  if (!node) return;
  delete_node(node->m_left);
  delete_node(node->m_right);
  delete node;
}
