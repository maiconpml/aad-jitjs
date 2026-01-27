#include "Instance.hpp"
#include "Parameters.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include "State.hpp"
#include "TabuList.hpp"
#include "Util.hpp"
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdint>
#include <tuple>
#include <utility>

using namespace std;

void Solver::swap_opers(State &state, const unsigned op1,
                        const unsigned op2) const {
#ifndef NDEBUG
  const Instance &inst = Instance::getInstance();
  assert(inst.operToM[op1] == inst.operToM[op2]);
  assert(op1 < inst.O);
  assert(op2 < inst.O);
#endif // NDEBUG
  assert(op1 * op2);
  if (op1 == op2)
    return;
  if (state.mach[op1] == op2) {
    rm_insert_oper_after(state, op1, op2);
  } else if (state.mach[op2] == op1) {
    rm_insert_oper_after(state, op2, op1);
  } else {
    unsigned _machOp1 = state._mach[op1], machOp1 = state.mach[op1];
    rm_insert_oper_after(state, op1, op2);
    if (_machOp1)
      rm_insert_oper_after(state, op2, _machOp1);
    else {
      rm_insert_oper_befor(state, op2, machOp1);
    }
  }
}

void Solver::rm_insert_oper_after(State &state, const unsigned op1,
                                  const unsigned op2) const {
#ifndef NDEBUG
  const Instance &inst = Instance::getInstance();
  assert(inst.operToM[op1] == inst.operToM[op2]);
  assert(op1 < inst.O);
  assert(op2 < inst.O);
#endif // NDEBUG
  assert(op1);
  assert(op2);
  if (op1 == op2)
    return;
  unsigned machOp1 = state.mach[op1], _machOp1 = state._mach[op1], machOp2;
  if (machOp1)
    state._mach[machOp1] = _machOp1;
  if (_machOp1)
    state.mach[_machOp1] = machOp1;
  machOp2 = state.mach[op2];
  if (machOp2)
    state._mach[machOp2] = op1;
  state.mach[op2] = op1;
  state._mach[op1] = op2;
  state.mach[op1] = machOp2;
}

void Solver::rm_insert_oper_befor(State &state, const unsigned op1,
                                  const unsigned op2) const {
#ifndef NDEBUG
  const Instance &inst = Instance::getInstance();
  assert(inst.operToM[op1] == inst.operToM[op2]);
  assert(op1 < inst.O);
  assert(op2 < inst.O);
#endif // NDEBUG
  assert(op1);
  assert(op2);
  if (op1 == op2)
    return;
  unsigned machOp1 = state.mach[op1], _machOp1 = state._mach[op1], _machOp2;
  if (machOp1)
    state._mach[machOp1] = _machOp1;
  if (_machOp1)
    state.mach[_machOp1] = machOp1;
  _machOp2 = state._mach[op2];
  if (_machOp2)
    state.mach[_machOp2] = op1;
  state._mach[op2] = op1;
  state.mach[op1] = op2;
  state._mach[op1] = _machOp2;
}

bool Solver::is_swap_tabu(TabuList &tList, State &state,
                          pair<unsigned, unsigned> &move) const {
  return tList.isTabu(state._mach[move.first], move.second) ||
         tList.isTabu(state._mach[move.second], move.first);
}

bool Solver::is_insert_tabu(TabuList &tList, State &state,
                            pair<unsigned, unsigned> &move,
                            MoveType type) const {
  switch (type) {
  case MoveType::BEFORE:
    return tList.isTabu(state._mach[move.second], move.first);
  case MoveType::AFTER:
    return tList.isTabu(move.second, move.first);
  };
  return false;
}

int Solver::get_swap_tabu_age(TabuList &tList, State &state,
                              pair<unsigned, unsigned> &move) const {
  return min(tList.age(state._mach[move.first], move.second),
             tList.age(state._mach[move.second], move.first));
}

int Solver::get_insert_tabu_age(TabuList &tList, State &state,
                                pair<unsigned, unsigned> &move,
                                MoveType type) const {
  switch (type) {
  case MoveType::BEFORE:
    return tList.age(state._mach[move.second], move.first);
  case MoveType::AFTER:
    return tList.age(move.second, move.first);
  };
  return -1;
}

void Solver::update_tabulist_swap(TabuList &tList, State &state,
                                  pair<unsigned, unsigned> &move,
                                  bool areAllMovesTabu) const {
  SchedPtr sched = get_sched_by_param();
  unsigned _machFirst = state._mach[move.first],
           _machSecond = state._mach[move.second];
  swap_opers(state, move.first, move.second);
  (this->*sched)(state);
  if (areAllMovesTabu) {
    if (tList.isTabu(state._mach[move.first], move.first))
      tList.passTime(tList.timeToLeave(state._mach[move.first], move.first));
    if (tList.isTabu(state._mach[move.second], move.second))
      tList.passTime(tList.timeToLeave(state._mach[move.second], move.second));
  }
  tList.insert(_machFirst, move.first);
  tList.insert(_machSecond, move.second);
}

void Solver::update_tabulist_insert(TabuList &tList, State &state,
                                    pair<unsigned, unsigned> &move,
                                    MoveType type, bool areAllMovesTabu) const {
  SchedPtr sched = get_sched_by_param();
  unsigned _machFirst = state._mach[move.first],
           _machSecond = state._mach[move.second];
  switch (type) {
  case Solver::MoveType::BEFORE:
    rm_insert_oper_befor(state, move.first, move.second);
    break;
  case Solver::MoveType::AFTER:
    rm_insert_oper_after(state, move.first, move.second);
  }
  (this->*sched)(state);

  if (areAllMovesTabu)
    tList.passTime(tList.timeToLeave(state._mach[move.first], move.first));
  switch (type) {
  case MoveType::BEFORE:
    tList.insert(_machSecond, move.first);
    break;
  case MoveType::AFTER:
    tList.insert(_machFirst, move.first);
    break;
  case MoveType::SWAP:
    break;
  }
}

double Solver::evaluate_swap(State &state,
                             pair<unsigned, unsigned> &move) const {
  SchedPtr sched = get_sched_by_param();

  unsigned i = move.first, j = move.second;
  double cost = DBL_MAX;
  swap_opers(state, i, j);
  if (!(this->*sched)(state)) {
    assert(validate_state(state));
    cost = state.penalties;
  }
  swap_opers(state, j, i);
  return cost;
}

double Solver::evaluate_insert(State &state, pair<unsigned, unsigned> &move,
                               Solver::MoveType type) const {
  SchedPtr sched = get_sched_by_param();
  unsigned i = move.first, j = move.second;
  unsigned _machOp1 = state._mach[i], machOp1 = state.mach[i];
  double cost = DBL_MAX;

  // do move
  switch (type) {
  case Solver::MoveType::BEFORE:
    rm_insert_oper_befor(state, i, j);
    break;
  case Solver::MoveType::AFTER:
    rm_insert_oper_after(state, i, j);
  }
  // if valid, get cost
  if (!(this->*sched)(state)) {
    assert(validate_state(state));
    cost = state.penalties;
  }
  // undo move
  if (_machOp1)
    rm_insert_oper_after(state, i, _machOp1);
  else
    rm_insert_oper_befor(state, i, machOp1);

  return cost;
}

void Solver::run_tabu_search_swap(State &state, TabuList &tList) const {
  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  double initStatePenalties = state.penalties;
  pair<unsigned, unsigned> oldestTabuMove(0, 0);
  int oldestTabuMoveAge = -1;
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  unsigned chosenMoveIndex = 0;
  pair<unsigned, unsigned> move;

  if (paramTraversing == Parameters::NHoodTraversing::FI)
    shuffle(_cands.begin(), _cands.end(), Random::getEngine());

  bool isTabu = false, isAspiration = false;
  int curMoveTabuAge = -1;
  for (unsigned i = 0; i < _cands.size(); ++i) {
    move = make_pair(get<0>(_cands[i]), get<1>(_cands[i]));
    double curPenal = evaluate_swap(state, move);

    if (curPenal < bestMovePenalties) {
      isTabu = is_swap_tabu(tList, state, move);
      if (isTabu) {
        curMoveTabuAge = get_swap_tabu_age(tList, state, move);
        isAspiration = curPenal < best.penalties;
      }

      if (!isTabu || isAspiration) {
        bestMove = move;
        bestMovePenalties = curPenal;
        chosenMoveIndex = i;

        if (paramTraversing == Parameters::NHoodTraversing::FI &&
            curPenal < initStatePenalties)
          break;
      } else if (curMoveTabuAge > oldestTabuMoveAge) {
        oldestTabuMoveAge = curMoveTabuAge;
        oldestTabuMove = move;
        chosenMoveIndex = i;
      }
    }
    assert(state == debugState);
  }
  assert(state == debugState);
  if (bestMovePenalties == DBL_MAX)
    update_tabulist_swap(tList, state, oldestTabuMove, true);
  else
    update_tabulist_swap(tList, state, bestMove, false);

  Util::rm_element_swap(_cands, chosenMoveIndex);
}

void Solver::run_tabu_search_insert(State &state, TabuList &tList) const {
  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  double initStatePenalties = state.penalties;
  pair<unsigned, unsigned> oldestTabuMove(0, 0);
  int oldestTabuMoveAge = -1;
  MoveType oldestMoveType;
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  MoveType bestMoveType;
  unsigned chosenMoveIndex = 0;
  pair<unsigned, unsigned> move;

  if (paramTraversing == Parameters::NHoodTraversing::FI)
    shuffle(_cands.begin(), _cands.end(), Random::getEngine());

  bool isTabu = false, isAspiration = false;
  int curMoveTabuAge = -1;
  for (unsigned i = 0; i < _cands.size(); ++i) {
    move = make_pair(get<0>(_cands[i]), get<1>(_cands[i]));
    MoveType type = get<2>(_cands[i]);

    double curPenal = evaluate_insert(state, move, type);

    if (curPenal < bestMovePenalties) {
      isTabu = is_insert_tabu(tList, state, move, type);
      if (isTabu) {
        curMoveTabuAge = get_insert_tabu_age(tList, state, move, type);
        isAspiration = curPenal < best.penalties;
      }

      if (!isTabu || isAspiration) {
        bestMove = move;
        bestMovePenalties = curPenal;
        bestMoveType = type;
        chosenMoveIndex = i;
        if (paramTraversing == Parameters::NHoodTraversing::FI &&
            curPenal < initStatePenalties)
          break;
      } else if (curMoveTabuAge > oldestTabuMoveAge) {
        oldestTabuMoveAge = curMoveTabuAge;
        oldestTabuMove = move;
        oldestMoveType = type;
        chosenMoveIndex = i;
      }
    }
    assert(state == debugState);
  }

  assert(state == debugState);
  if (bestMovePenalties == DBL_MAX)
    update_tabulist_insert(tList, state, oldestTabuMove, oldestMoveType, true);
  else
    update_tabulist_insert(tList, state, bestMove, bestMoveType, false);

  Util::rm_element_swap(_cands, chosenMoveIndex);
}

void Solver::cands_swap_adjacent(State &state) const {
  _cands.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.mach[o])
      _cands.push_back(make_tuple(o, state.mach[o], MoveType::SWAP));
  }
}

void Solver::nhood_tabu_swap_adjacent(State &state, TabuList &tList) const {
  run_tabu_search_swap(state, tList);
}

void Solver::cands_swap_random(State &state) const {
  const Instance &inst = Instance::getInstance();
  _cands.clear();
  unsigned op1, op2;
  for (unsigned i = 0; i < 2 * state.mach.size(); ++i) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    _cands.push_back(make_tuple(op1, op2, MoveType::SWAP));
  }
}

void Solver::nhood_tabu_swap_random(State &state, TabuList &tList) const {
  run_tabu_search_swap(state, tList);
}

void Solver::cands_rm_insert_random(State &state) const {
  const Instance &inst = Instance::getInstance();
  _cands.clear();
  unsigned op1, op2;
  for (unsigned i = 0; i < 2 * state.mach.size(); ++i) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    _cands.push_back(make_tuple(op1, op2, MoveType::AFTER));
  }
}

void Solver::nhood_tabu_rm_insert_random(State &state, TabuList &tList) const {
  run_tabu_search_insert(state, tList);
}

void Solver::cands_swap_earl_late(State &state) const {
  const Instance &inst = Instance::getInstance();

  _cands.clear();
  vector<bool> checked(inst.O, false);
  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] == inst.deadlines[o])
      continue;

    bool isCurOpEarly = state.starts[o] + inst.P[o] < inst.deadlines[o];
    while (o && !checked[o]) {
      checked[o] = true;

      // if operation is early (tardy) and it's machine successor
      // (predecessor) exists and is adjecent with o, that is, starts
      // (ends) exactly when o ends (starts), choose successor
      // (predecessor) as swap candidate
      unsigned swapOpCand = 0;
      if (isCurOpEarly && state.mach[o] &&
          state.starts[o] + inst.P[o] == state.starts[state.mach[o]]) {
        swapOpCand = state.mach[o];
      } else if (!isCurOpEarly && state._mach[o] &&
                 state.starts[o] ==
                     state.starts[state._mach[o]] + inst.P[state._mach[o]]) {
        swapOpCand = state._mach[o];
      }

      if (swapOpCand) {
        _cands.push_back(make_tuple(o, swapOpCand, MoveType::SWAP));
        break;
      }

      if (isCurOpEarly) {
        o = inst.job[o];
      } else {
        o = inst._job[o];
      }
    }
  }
}

void Solver::nhood_tabu_swap_earl_late(State &state, TabuList &tList) const {
  run_tabu_search_swap(state, tList);
}

void Solver::cands_insert_earl_late(State &state) const {
  const Instance &inst = Instance::getInstance();

  _cands.clear();

  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  state.find_blocks(machBlocks, opToBlock);

  unsigned insertOpCand;
  for (unsigned o = 1; o < inst.O; ++o) {
    bool isCurOpEarly = state.starts[o] + inst.P[o] < inst.deadlines[o];

    // impossible to perform any move
    if (machBlocks[opToBlock[o].first].size() == 1)
      continue;

    if (isCurOpEarly) {
      insertOpCand = machBlocks[opToBlock[o].first].back();
      // if operation has a job successor and the last operation of current
      // block starts after this job successor, search for first operation on
      // block that starts before JS from last until o
      if (inst.job[o] &&
          state.starts[insertOpCand] > state.starts[inst.job[o]]) {
        int j = machBlocks[opToBlock[o].first].size() - 1;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[o].first][j] != o &&
               state.starts[machBlocks[opToBlock[o].first][j]] >
                   state.starts[inst.job[o]]) {
          insertOpCand = machBlocks[opToBlock[o].first][j--];
        }
      }
      if (o == insertOpCand)
        continue;
      _cands.push_back(make_tuple(o, insertOpCand, MoveType::AFTER));
    } else {
      insertOpCand = machBlocks[opToBlock[o].first].front();
      // if operation has a job predecessor and the first operation of current
      // block ends before this job successor, search for first operation on
      // block that starts before JS from first until o
      if (inst._job[o] &&
          state.starts[insertOpCand] + inst.P[insertOpCand] <
              state.starts[inst._job[o]] + inst.P[inst._job[o]]) {
        int j = 0;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[o].first][j] != o &&
               state.starts[machBlocks[opToBlock[o].first][j]] +
                       inst.P[machBlocks[opToBlock[o].first][j]] <
                   state.starts[inst._job[o]] + inst.P[inst._job[o]]) {
          insertOpCand = machBlocks[opToBlock[o].first][j++];
        }
      }
      if (o == insertOpCand)
        continue;
      _cands.push_back(make_tuple(o, insertOpCand, MoveType::BEFORE));
    }
  }
}

void Solver::nhood_tabu_insert_earl_late(State &state, TabuList &tList) const {
  run_tabu_search_insert(state, tList);
}

void Solver::cands_oper_critical(State &state) const {
  const Instance &inst = Instance::getInstance();

  _cands.clear();
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  state.find_blocks(machBlocks, opToBlock);

  unsigned curOp;
  vector<bool> isOpChecked(inst.O, false);
  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] <= inst.deadlines[o])
      continue;
    curOp = o;
    unsigned curBlock;
    unsigned curOpPos;
    while (state._mach[curOp] || inst._job[curOp]) {
      curBlock = opToBlock[curOp].first;
      curOpPos = opToBlock[curOp].second;
      if (isOpChecked[machBlocks[curBlock][curOpPos]])
        break;
      if (curOpPos > 0) {
        _cands.push_back(make_tuple(machBlocks[curBlock][curOpPos],
                                    machBlocks[curBlock][curOpPos - 1],
                                    MoveType::SWAP));
        isOpChecked[machBlocks[curBlock][curOpPos]] = true;
      }
      if (machBlocks[curBlock].size() > 1 &&
          isOpChecked[machBlocks[curBlock][1]])
        break;
      if (curOpPos > 1) {
        _cands.push_back(make_tuple(machBlocks[curBlock][0],
                                    machBlocks[curBlock][1], MoveType::SWAP));
        isOpChecked[machBlocks[curBlock][1]] = true;
      }

      curOp = inst._job[machBlocks[curBlock][0]];
    }
  }
}

void Solver::nhood_tabu_oper_critical(State &state, TabuList &tList) const {
  run_tabu_search_swap(state, tList);
}

void Solver::cands_oper_critical_alt(State &state) const {
  const Instance &inst = Instance::getInstance();

  _cands.clear();
  unsigned curOp;
  for (unsigned o = 1; o < inst.O; ++o) {
    vector<unsigned> opCritic;

    curOp = o;
    opCritic.push_back(o);
    while (inst._job[curOp] != 0 || state._mach[curOp] != 0) {

      while (state._mach[curOp] &&
             state.starts[state._mach[curOp]] + inst.P[state._mach[curOp]] >
                 state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
        opCritic.push_back(state._mach[curOp]);
        curOp = state._mach[curOp];
      }
      if (opCritic.size() > 1)
        _cands.push_back(make_tuple(opCritic[0], opCritic[1], MoveType::SWAP));
      if (opCritic.size() > 2)
        _cands.push_back(make_tuple(opCritic[opCritic.size() - 1],
                                    opCritic[opCritic.size() - 2],
                                    MoveType::SWAP));

      curOp = inst._job[curOp];
      opCritic.clear();
    }
  }
}

void Solver::nhood_tabu_oper_critical_alt(State &state, TabuList &tList) const {
  run_tabu_search_swap(state, tList);
}
