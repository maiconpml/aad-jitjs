#include "TabuTrio.hpp"

TabuTrio::TabuTrio() : isDummy(true) {}

TabuTrio::TabuTrio(
    const State &_state,
    const vector<tuple<unsigned, unsigned, Solver::MoveType>> &_cands,
    const TabuList &_tabuList)
    : isDummy(false), state(_state), nhoodMoves(_cands), tabuList(_tabuList) {}

TabuTrio::~TabuTrio() {}
