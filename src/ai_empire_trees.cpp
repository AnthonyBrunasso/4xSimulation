#include "ai_empire_trees.h"

#include "ai_empire_decisions.h"
#include "ai_empire_evaluations.h"
#include "dtree.h"

DTree& empire_trees::get_primitive_macro() {
  // ok to leak these on shutdown for now
  static DTree s_primitive_macro([]() -> DNode* {
    // Create macro tree. Check if the player needs to colonize.
    DNode* node = new DNode(nullptr, &empire_evaluations::get_colonize());

    // If they need colonization: create a city.
    node->m_right = new DNode(&empire_decisions::get_settle(), nullptr);
    // city exists: evaluate the production needs of the city.
    node->m_left = new DNode(nullptr, &empire_evaluations::get_produce());
    DNode*& produce_eval = node->m_left;
    // Construct something in the city.
    produce_eval->m_right = new DNode(&empire_decisions::get_construct(), nullptr);
    
    return node;
  }());

  return s_primitive_macro;
}

DTree& empire_trees::get_primitive_micro(TurnState* state) {
  // ok to leak these on shutdown for now
  static DTree s_primitive_micro([]() -> DNode*
  {
    // Create micro tree.
    DNode* root = new DNode(nullptr, &empire_evaluations::has_idle_military());
    DNode* eval_fortified = new DNode(nullptr, &empire_evaluations::has_fortified());
    DNode* dvi = new DNode(nullptr, &empire_evaluations::defender_vs_idle());
    DNode* tva = new DNode(nullptr, &empire_evaluations::threatened_vs_available());
    DNode* fcvgc = new DNode(nullptr, &empire_evaluations::find_city_vs_goto_city());
    DNode* pvs = new DNode(nullptr, &empire_evaluations::pillage_vs_siege());
    DNode* ava = new DNode(nullptr, &empire_evaluations::approach_vs_attack());
    DNode* avs = new DNode(nullptr, &empire_evaluations::approach_vs_siege());
    DNode* avp = new DNode(nullptr, &empire_evaluations::approach_vs_pillage());

    DNode* decide_endturn = new DNode(&empire_decisions::decide_endturn(), nullptr);
    DNode* decide_fortify = new DNode(&empire_decisions::decide_fortify(), nullptr);
    DNode* decide_approach = new DNode(&empire_decisions::decide_approach(), nullptr);
    DNode* decide_attack = new DNode(&empire_decisions::decide_attack(), nullptr);
    DNode* decide_siege = new DNode(&empire_decisions::decide_siege(), nullptr);
    DNode* decide_pillage = new DNode(&empire_decisions::decide_pillage(), nullptr);
    DNode* decide_wander = new DNode(&empire_decisions::decide_wander(), nullptr);

    root->m_left = decide_endturn;
    //root->m_right = dvi;
    root->m_right = tva;
    //1
    //dvi->m_left = eval_fortified;
    //dvi->m_right = tva;
    //2
    eval_fortified->m_left = decide_endturn;
    eval_fortified->m_right = decide_fortify;
    tva->m_left = ava;
    tva->m_right = fcvgc;
    //3
    ava->m_left = decide_approach;
    ava->m_right = decide_attack;
    fcvgc->m_left = decide_wander;
    fcvgc->m_right = pvs;
    //4
    pvs->m_left = avp;
    pvs->m_right = avs;
    //5
    avp->m_left = decide_approach;
    avp->m_right = decide_pillage;
    avs->m_left = decide_approach;
    avs->m_right = decide_siege;

    return root;
  }());
  
  s_primitive_micro.m_state = state;

  return s_primitive_micro;
}

void empire_trees::shutdown() {
}
