#include "Instance.hpp"
#include "Parameters.hpp"
#include "Solver.hpp"
#include "Timer.hpp"
#include <utility>

void Solver::search_ls(State &initialSol) {
  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  NHoodPtr nhood = get_nhood_by_param();

  best = initialSol;
  State curState = initialSol, neighbor;
  pair<unsigned, unsigned> move;
  while (!Timer::isTimeExceeded(params.maxMilli)) {

    (this->*nhood)(curState, move);

    if (neighbor.penalties < best.penalties) {
      best = neighbor;
      best.millisecsFound = Timer::elapsedMs();
    } else {
      break;
    }
  }
}
