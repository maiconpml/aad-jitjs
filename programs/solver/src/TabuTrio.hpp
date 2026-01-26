#pragma once

#include "Solver.hpp"
#include "State.hpp"
#include "TabuList.hpp"
class TabuTrio {
public:
  TabuTrio();

  TabuTrio(const State &_state,
           const vector<tuple<unsigned, unsigned, Solver::MoveType>> &_cands,
           const TabuList &_tabuList);

  ~TabuTrio();

  bool isDummy;
  State state;
  // candidate neighbor moves of state
  vector<tuple<unsigned, unsigned, Solver::MoveType>> nhoodMoves;
  TabuList tabuList;
};
