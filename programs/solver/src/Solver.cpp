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

  vector<unsigned> topo;

  if (topo_sort(state, topo)) {
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

bool Solver::topo_sort(const State &state, vector<unsigned int> &topo) {
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

  while (!q.empty()) {
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

  assert(topo.size() <= inst.job.size() - 1);

  return topo.size() < inst.job.size() - 1;
}
