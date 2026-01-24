#include "Solver.hpp"
#include "Instance.hpp"
#include "Parameters.hpp"
#include "Timer.hpp"
#include <stdexcept>
#include <vector>

vector<unsigned> Solver::indeg;
vector<unsigned> Solver::q;

Solver::Solver(const Parameters &parameters) : params(parameters) {}

bool Solver::validate_state(const State &state) const {
  const Instance &inst = Instance::getInstance();

  if (topo_sort(state)) {
    return false;
  }

  for (unsigned o : q) {
    if (inst.job[o] && state.starts[o] + inst.P[o] > state.starts[inst.job[o]])
      return false;
    if (state.mach[o] &&
        state.starts[o] + inst.P[o] > state.starts[state.mach[o]])
      return false;
  }

  return true;
}

State Solver::solve() {
  Timer::start();

  switch (params.initialSolution) {
  case Parameters::InitialSolution::GT:
    initial_gt(best);
    break;
  case Parameters::InitialSolution::CONSTR:
    initial_constr_dispatch(best);
    break;
  }
  best.millisecsFound = Timer::elapsedMs();

#ifndef NDEBUG
  if (!validate_state(best)) {
    throw runtime_error("Invalid Initial State!!!");
  }
#endif // NDEBUG

  switch (params.searchMethods[params.currentSearchMethod]) {
  case Parameters::SearchMethod::LS:
    search_ls(best);
    break;
  case Parameters::SearchMethod::ILS:
    break;
  case Parameters::SearchMethod::TABU:
    break;
  }

  if (!validate_state(best)) {
    throw runtime_error("Invalid State!!!");
  }
  return best;
}

bool Solver::topo_sort(const State &state) {
  const Instance &inst = Instance::getInstance();

  if (indeg.size() < inst.O) {
    indeg.resize(inst.O);
  }
  fill(indeg.begin(), indeg.end(), 0);

  if (q.capacity() < inst.O) {
    q.reserve(inst.O);
  }
  q.clear();

  unsigned curOp;
  unsigned newOp;
  unsigned head = 0;

  for (unsigned o = 1; o < inst.job.size(); o++) {
    if (inst._job[o] != 0)
      ++indeg[o];
    if (state._mach[o] != 0)
      ++indeg[o];
    if (indeg[o] == 0) {
      q.push_back(o);
    }
  }
  assert(!q.empty());

  while (head < q.size()) {
    curOp = q[head++];

    newOp = inst.job[curOp];
    if (newOp) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp])
        q.push_back(newOp);
    }

    newOp = state.mach[curOp];
    if (newOp) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp])
        q.push_back(newOp);
    }
  }

  assert(head <= inst.O);

  return head < inst.O - 1;
}

Solver::NHoodPtr Solver::get_nhood_by_param() const {

  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
    return &Solver::nhood_swap_adjacent;
  case Parameters::Neighborhood::SWAP_PENAL:
    return &Solver::nhood_swap_earl_late;
  case Parameters::Neighborhood::SWAP_RAND:
    return &Solver::nhood_swap_random;
  case Parameters::Neighborhood::INSERT_RAND:
    return &Solver::nhood_rm_insert_random;
  case Parameters::Neighborhood::INSERT_PENAL:
    return &Solver::nhood_insert_earl_late;
  case Parameters::Neighborhood::CRITICAL_OPER:
    return &Solver::nhood_oper_critical;
  case Parameters::Neighborhood::CRITICAL_OPER_ALT:
    return &Solver::nhood_oper_critical_alt;
  }

  return NULL;
}
