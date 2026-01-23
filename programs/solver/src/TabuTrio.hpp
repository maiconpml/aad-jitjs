#pragma once

#include "State.hpp"
#include "TabuList.hpp"
class TabuTrio {
public:
  TabuTrio();

  TabuTrio(const State &_state, const vector<pair<unsigned, unsigned>> &_cands,
           const TabuList &_tabuList);

  ~TabuTrio();

  bool isDummy;
  State state;
  // candidate neighbor moves of state
  vector<pair<unsigned, unsigned>> nhoodMoves;
  TabuList tabuList;
};
