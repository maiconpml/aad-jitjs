#include "Solver.hpp"
#include "Instance.hpp"
#include "Parameters.hpp"
#include "Util.hpp"
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

  switch (params.initialSolution) {
  case Parameters::InitialSolution::GT:
    initial_gt(best);
    break;
  case Parameters::InitialSolution::CONSTR:
    initial_constr_dispatch(best);
    break;
  }

#ifndef NDEBUG
  if (!validate_state(best)) {
    throw runtime_error("Invalid State!!!");
  }
#endif // NDEBUG

  switch (params.searchMethods[params.currentSearchMethod]) {
  case Parameters::SearchMethod::LS:
    search_ls(best);
    break;
  }

  if (!validate_state(best)) {
    throw runtime_error("Invalid State!!!");
  }
  return best;
}
