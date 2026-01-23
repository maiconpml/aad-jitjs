#include "TabuTrio.hpp"

TabuTrio::TabuTrio() : isDummy(true) {}

TabuTrio::TabuTrio(const State &_state,
                   const vector<pair<unsigned, unsigned>> &_cands,
                   const TabuList &_tabuList)
    : isDummy(false), state(_state), nhoodMoves(_cands), tabuList(_tabuList) {}

TabuTrio::~TabuTrio() {}
