#include "Parameters.hpp"
#include "Solver.hpp"
#include "Timer.hpp"

void Solver::search_ls(State &initialSol) {
  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  using NHoodPtr = void (Solver::*)(const State &, State &) const;
  NHoodPtr nhood;

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
    nhood = &Solver::nhood_swap_adjacent;
    break;
  case Parameters::Neighborhood::SWAP_PENAL:
    nhood = &Solver::nhood_swap_earl_late;
    break;
  case Parameters::Neighborhood::SWAP_RAND:
    nhood = &Solver::nhood_swap_random;
    break;
  case Parameters::Neighborhood::INSERT_RAND:
    nhood = &Solver::nhood_rm_insert_random;
    break;
  case Parameters::Neighborhood::INSERT_PENAL:
    nhood = &Solver::nhood_insert_earl_late;
    break;
  case Parameters::Neighborhood::CRITICAL_OPER:
    nhood = &Solver::nhood_oper_critical;
    break;
  case Parameters::Neighborhood::CRITICAL_OPER_ALT:
    nhood = &Solver::nhood_oper_critical_alt;
    break;
  }

  best = initialSol;
  while (!Timer::isTimeExceeded(params.maxMilli)) {

    State neighbor;

    (this->*nhood)(best, neighbor);

    if (neighbor.penalties < best.penalties) {
      best = neighbor;
      best.millisecsFound = Timer::elapsedMs();
    } else {
      break;
    }
  }
}
