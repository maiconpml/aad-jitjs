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
