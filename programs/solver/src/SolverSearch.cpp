#include "Parameters.hpp"
#include "Solver.hpp"
#include "TabuCycleDetector.hpp"
#include "TabuJumpList.hpp"
#include "TabuList.hpp"
#include "TabuTrio.hpp"
#include "Timer.hpp"
#include <cassert>
#include <iostream>

void Solver::search_ls(State &state) {
  NHoodLSPtr nhood = get_nhood_ls_by_param();

  State curState = state;
  while (!Timer::isTimeExceeded(params.maxMilli)) {

    (this->*nhood)(curState);

    if (curState.penalties < state.penalties) {
      state = curState;
      state.millisecsFound = Timer::elapsedMs();
    } else {
      break;
    }
  }
}


void Solver::search_tabu(State &state) {
  NSTabuPtr nsp = get_ns_tabu_by_param();

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

  State curState = state;
  State auxState;

  while (!Timer::isTimeExceeded(params.maxMilli)) {

    cycling = cycleDetector.detect(curState.penalties, isNewBest);

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
      (this->*nsp)(curState, tList);
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

    if (curState.penalties < state.penalties) {
      assert(validate_state(curState));
      isNewBest = true;
      state = curState;
      state.millisecsFound = Timer::elapsedMs();

      curJumpLimit = params.initialJumpLimit;
    } else {
      ++noImproveIters;
    }
  }
}

void Solver::search_ils(State &state) {

  if (params.searchMethods[params.currentSearchMethod] ==
      Parameters::SearchMethod::ILS) {
    params.currentSearchMethod++;
  }
  SearchPtr search = get_search_by_param();
  State curState;

  unsigned paramPertStr = params.perturbationStrength;
  int n = params.nHoods.size();

  while (!Timer::isTimeExceeded(params.maxMilli) && params.currentNHood < n) {

    curState = state;

    pert_swap_adj_random(curState, paramPertStr);

    (this->*search)(curState);

    if (curState.penalties < state.penalties) {
      state = curState;
      params.currentNHood = 0;
    } else if (n > 1) {
      ++params.currentNHood;
    }
  }
}
