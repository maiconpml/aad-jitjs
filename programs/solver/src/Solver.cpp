#include "Solver.hpp"
#include "Instance.hpp"
#include "Parameters.hpp"
#include "Timer.hpp"
#include <stdexcept>
#include <vector>

vector<unsigned> Solver::indeg;
vector<unsigned> Solver::q;
vector<unsigned> Solver::_ops;
vector<tuple<unsigned, unsigned, Solver::MoveType>> Solver::_cands;

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

  SearchPtr search = get_search_by_param();

  (this->*search)(best);

  sched_cplex(best);
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

Solver::NHoodLSPtr Solver::get_nhood_ls_by_param() const {

  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
    return &Solver::nhood_ls_swap_adjacent;
  case Parameters::Neighborhood::SWAP_PENAL:
    return &Solver::nhood_ls_swap_earl_late;
  case Parameters::Neighborhood::SWAP_RAND:
    return &Solver::nhood_ls_swap_random;
  case Parameters::Neighborhood::INSERT_RAND:
    return &Solver::nhood_ls_rm_insert_random;
  case Parameters::Neighborhood::INSERT_PENAL:
    return &Solver::nhood_ls_insert_earl_late;
  case Parameters::Neighborhood::CRITICAL_OPER:
    return &Solver::nhood_ls_oper_critical;
  case Parameters::Neighborhood::CRITICAL_OPER_ALT:
    return &Solver::nhood_ls_oper_critical_alt;
  case Parameters::Neighborhood::RELAX_2:
    return &Solver::nhood_ls_relax_2;
  }
  return NULL;
}

Solver::NSTabuPtr Solver::get_ns_tabu_by_param() const {

  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
  case Parameters::Neighborhood::SWAP_PENAL:
  case Parameters::Neighborhood::SWAP_RAND:
  case Parameters::Neighborhood::CRITICAL_OPER:
  case Parameters::Neighborhood::CRITICAL_OPER_ALT:
    return &Solver::run_ns_tabu_swap;
  case Parameters::Neighborhood::INSERT_RAND:
  case Parameters::Neighborhood::INSERT_PENAL:
    return &Solver::run_ns_tabu_insert;
  }
  return NULL;
}

Solver::CandsPtr Solver::get_cands_by_param() const {
  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
    return &Solver::cands_swap_adjacent;
  case Parameters::Neighborhood::SWAP_PENAL:
    return &Solver::cands_swap_earl_late;
  case Parameters::Neighborhood::SWAP_RAND:
    return &Solver::cands_swap_random;
  case Parameters::Neighborhood::INSERT_RAND:
    return &Solver::cands_rm_insert_random;
  case Parameters::Neighborhood::INSERT_PENAL:
    return &Solver::cands_insert_earl_late;
  case Parameters::Neighborhood::CRITICAL_OPER:
    return &Solver::cands_oper_critical;
  case Parameters::Neighborhood::CRITICAL_OPER_ALT:
    return &Solver::cands_oper_critical_alt;
  }
  return NULL;
}

Solver::SearchPtr Solver::get_search_by_param() const {
  Parameters::SearchMethod paramSearch =
      params.searchMethods[params.currentSearchMethod];

  switch (paramSearch) {
  case Parameters::SearchMethod::LS:
    return &Solver::search_ls;
  case Parameters::SearchMethod::TABU:
    return &Solver::search_tabu;
  case Parameters::SearchMethod::ILS:
    return &Solver::search_ils;
  }
  return NULL;
}
