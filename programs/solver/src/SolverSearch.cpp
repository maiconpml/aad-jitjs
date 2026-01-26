#include "Instance.hpp"
#include "Parameters.hpp"
#include "Solver.hpp"
#include "TabuCycleDetector.hpp"
#include "TabuJumpList.hpp"
#include "TabuList.hpp"
#include "TabuTrio.hpp"
#include "Timer.hpp"
#include <cassert>
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

void Solver::search_tabu(State &initialSol) {
  const Instance &inst = Instance::getInstance();

  NHoodTabuPtr nhood = get_nhood_tabu_by_param();

  CandsPtr cands = get_cands_by_param();

  TabuCycleDetector cycleDetector(params.maxD, params.maxC);

  TabuList tList(params.tenure);
  TabuList auxTL;
  TabuJumpList tJumpL(params.jumpListSize);
  TabuTrio tt;

  bool cycling = false;
  bool isNewBest = true;
  bool jumped = false;
  bool emptyNHood = false;
  unsigned curJumpLimit = params.initialJumpLimit;
  unsigned noImproveIters = 0;

  best = initialSol;
  State curState = initialSol;
  State auxState;

  while (!Timer::isTimeExceeded(params.maxMilli)) {

    cycling = cycleDetector.detect(initialSol.penalties, isNewBest);

    if (noImproveIters > curJumpLimit || cycling || emptyNHood) {

      curJumpLimit -= curJumpLimit / params.decreaseDivisor;

      tt = tJumpL.pop();

      if (tt.isDummy)
        return;

      _cands = tt.nhoodMoves;
      curState = tt.state;
      tList = tt.tabuList;

      jumped = true;
      assert(!_cands.empty());
    } else {
      (this->*cands)(curState);
      jumped = false;
    }

    if (isNewBest) {
      auxState = curState;
      auxTL = tList;
    }

    if (!_cands.empty())
      (this->*nhood)(curState, tList);
    else
      emptyNHood = true;

    if (isNewBest) {
      if (!_cands.empty())
        tJumpL.push(TabuTrio(auxState, _cands, auxTL));
      isNewBest = false;
    }

    if (jumped) {
      tJumpL.updateCands(_cands);
    }

    if (curState.penalties < best.penalties) {
      isNewBest = true;
      best = curState;

      curJumpLimit = params.initialJumpLimit;
    } else {
      ++noImproveIters;
    }
  }
}
