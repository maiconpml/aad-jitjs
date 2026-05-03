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

  unsigned stTime = Timer::elapsedMs();
  State curState = state;
  while (!Timer::isTimeExceeded(
      min(stTime + params.internalSearchTime, params.maxMilli))) {

    if (params.hyb_sched) {
      if (curState.ePenalty > 0 &&
          ((curState.tPenalty - curState.ePenalty) / curState.ePenalty) > 0.1)
        params.sched = Parameters::Scheduler::DELAYING;
      else
        params.sched = Parameters::Scheduler::EARLY;
    }

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

  unsigned stTime = Timer::elapsedMs();
  State curState = state;
  State auxState;

  if (curState.penalties < state.penalties) {
    state = curState;
    state.millisecsFound = Timer::elapsedMs();
  }
  while (!Timer::isTimeExceeded(
      min(stTime + params.internalSearchTime, params.maxMilli))) {

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

    if (params.hyb_sched) {
      if (curState.ePenalty > 0 &&
          ((curState.tPenalty - curState.ePenalty) / curState.ePenalty) > 0.1)
        params.sched = Parameters::Scheduler::DELAYING;
      else
        params.sched = Parameters::Scheduler::EARLY;
    }
    if (!_cands.empty())
      (this->*nsp)(curState, tList, state.penalties);
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
  State curState = state;

  (this->*search)(curState);

  if (curState.penalties < state.penalties) {
    state = curState;
    params.currentNHood = 0;
  }

  unsigned paramPertStr = params.perturbationStrength;
  int n = params.nHoods.size();

  while (!Timer::isTimeExceeded(params.maxMilli) && params.currentNHood < n) {

    curState = state;

    switch (params.perturbationType) {
    case Parameters::PerturbationType::SWAP_ADJ:
      pert_swap_adj_random(curState, paramPertStr);
      break;
    case Parameters::PerturbationType::RELAX_1:
      pert_relax_1(curState, paramPertStr);
      break;
    }

    (this->*search)(curState);

    if (curState.penalties < state.penalties) {
      state = curState;
      params.currentNHood = 0;
    } else {
      params.currentNHood = (params.currentNHood + 1) % n;
    }
  }
}
