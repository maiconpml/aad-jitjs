#include "Solver.hpp"
#include "Instance.hpp"
#include "Util.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

Solver::Solver(const Parameters &parameters) : params(parameters) {}

bool Solver::validate_state(const State &state) const {
  const Instance &inst = Instance::getInstance();

  vector<unsigned> topo(inst.O - 1);

  if (Util::topo_sort(inst.job, state.mach, inst._job, state._mach, topo)) {
    return false;
  }

  for (unsigned o : topo) {
    if (inst.job[o] && state.starts[o] + inst.P[o] > state.starts[inst.job[o]])
      return false;
    if (state.mach[o] &&
        state.starts[o] + inst.P[o] > state.starts[state.mach[o]])
      return false;
  }

  return true;
}

State Solver::solve() {
  giffler_thompson(best);
  if (!validate_state(best)) {
    throw runtime_error("Invalid State!!!");
  }
  return best;
}
