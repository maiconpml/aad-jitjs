#include "Parameters.hpp"
#include "Solver.hpp"

void Solver::search_ls(State &initialSol) {
  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  using NHoodPtr = void (Solver::*)(const State &, State &) const;
  NHoodPtr nhood;

  switch (paramNHood) {
  case Parameters::Neighborhood::SWAP_ADJ:
    nhood = &Solver::nhood_swap_adjacent;
  }
  best = initialSol;
  while (true) {
    State neighbor;

    (this->*nhood)(best, neighbor);

    if (neighbor.penalties < best.penalties) {
      best = neighbor;
    } else {
      break;
    }
  }
}
