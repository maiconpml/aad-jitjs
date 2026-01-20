#include "State.hpp"
#include "Instance.hpp"
#include "Solver.hpp"
#include <cfloat>

State::State() : penalties(DBL_MAX) {}

double State::calc_penalties() {
  const Instance &inst = Instance::getInstance();
  unsigned curTard, curEarl;
  tPenalty = 0;
  ePenalty = 0;
  for (unsigned o = 1; o < starts.size(); ++o) {
    curEarl = inst.deadlines[o] - starts[o] + inst.P[o];
    curTard = starts[o] + inst.P[o] - inst.deadlines[o];
    if (curEarl > 0)
      ePenalty += curEarl * inst.earlCoefs[o];
    else if (curTard > 0)
      tPenalty += curTard * inst.tardCoefs[o];
  }

  penalties = ePenalty + tPenalty;
  return penalties;
}

bool State::operator==(const State &s) {
  for (unsigned o = 1; o < mach.size(); ++o) {
    if (mach[o] != s.mach[o] || _mach[o] != s._mach[o])
      return false;
  }

  return true;
}
