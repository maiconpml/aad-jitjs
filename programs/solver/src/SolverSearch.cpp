#include "Instance.hpp"
#include "Parameters.hpp"
#include "Solver.hpp"
#include "TabuCycleDetector.hpp"
#include "TabuJumpList.hpp"
#include "TabuList.hpp"
#include "Timer.hpp"
#include <utility>

void Solver::search_ls(State &initialSol) {
  Parameters::Neighborhood paramNHood = params.nHoods[params.currentNHood];

  NHoodLSPtr nhood = get_nhood_ls_by_param();

  best = initialSol;
  State curState = initialSol, neighbor;
  pair<unsigned, unsigned> move;
  while (!Timer::isTimeExceeded(params.maxMilli)) {

    (this->*nhood)(curState);

    if (neighbor.penalties < best.penalties) {
      best = neighbor;
      best.millisecsFound = Timer::elapsedMs();
    } else {
      break;
    }
  }
}
