#include "Instance.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <cfloat>
#include <utility>

void Solver::nhood_ls_swap_adjacent(State &state) const {
  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    _ops.clear();
    for (unsigned o = 1; o < state.mach.size(); ++o) {
      _ops.push_back(o);
    }
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;

  unsigned i;
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (paramTraversing == Parameters::NHoodTraversing::FI)
      i = _ops[o - 1];
    else
      i = o;

    pair<unsigned, unsigned> move(make_pair(i, state.mach[i]));
    if (move.second) {
      double curPenal = evaluate_swap(state, move);
      if (curPenal < bestMovePenalties) {
        bestMove = move;
        if (paramTraversing == Parameters::NHoodTraversing::FI &&
            curPenal < initStatePenalties)
          break;
        bestMovePenalties = curPenal;
      }
      assert(state == debugState);
    }
  }
  swap_opers(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_swap_random(State &state) const {
  const Instance &inst = Instance::getInstance();

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;

  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    pair<unsigned, unsigned> move(make_pair(op1, op2));
    double curPenal = evaluate_swap(state, move);
    if (curPenal < bestMovePenalties) {
      bestMove = move;
      if (paramTraversing == Parameters::NHoodTraversing::FI &&
          curPenal < initStatePenalties)
        break;
      bestMovePenalties = curPenal;
    }
    assert(state == debugState);
  }
  swap_opers(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_rm_insert_random(State &state) const {
  const Instance &inst = Instance::getInstance();

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;

  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    pair<unsigned, unsigned> move(make_pair(op1, op2));
    double curPenal = evaluate_insert(state, move, MoveType::AFTER);
    if (curPenal < bestMovePenalties) {
      bestMove = move;
      if (paramTraversing == Parameters::NHoodTraversing::FI &&
          curPenal < initStatePenalties)
        break;
      bestMovePenalties = curPenal;
    }
    assert(state == debugState);
  }
  rm_insert_oper_after(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_swap_earl_late(State &state) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  // checked[o] == true means that operation o was already verified and if it
  // was possible this swap has already been tested
  vector<bool> checked(inst.O, false);

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] == inst.deadlines[o])
      checked[o] = true;
    else
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  unsigned curOp;
  for (unsigned o = 0; o < _ops.size(); ++o) {
    curOp = _ops[o];

    bool isCurOpEarly =
        state.starts[curOp] + inst.P[curOp] < inst.deadlines[curOp];

    // while operation exists and is not checked
    while (curOp && !checked[curOp]) {
      checked[curOp] = true;

      // if operation is early (tardy) and it's machine successor
      // (predecessor) exists and is adjecent with curOp, that is, starts
      // (ends) exactly when curOp ends (starts), choose successor
      // (predecessor) as swap candidate
      unsigned swapOpCand = 0;
      if (isCurOpEarly && state.mach[curOp] &&
          state.starts[curOp] + inst.P[curOp] ==
              state.starts[state.mach[curOp]]) {
        swapOpCand = state.mach[curOp];
      } else if (!isCurOpEarly && state._mach[curOp] &&
                 state.starts[curOp] == state.starts[state._mach[curOp]] +
                                            inst.P[state._mach[curOp]]) {
        swapOpCand = state._mach[curOp];
      }

      // if swap candidate was selected, execute swap, verify improvement on
      // generated neighbor and undo de swap movement
      if (swapOpCand) {
        pair<unsigned, unsigned> move(make_pair(curOp, swapOpCand));
        assert(validate_state(state));
        double curPenal = evaluate_swap(state, move);
        if (curPenal < bestMovePenalties) {
          bestMove = move;
          if (paramTraversing == Parameters::NHoodTraversing::FI &&
              curPenal < initStatePenalties) {
            swap_opers(state, bestMove.first, bestMove.second);
            (this->*get_sched_by_param())(state);
            return;
          }
          bestMovePenalties = curPenal;
        }
        assert(state == debugState);
        break;
      }

      // follow job sequence to find a swap candidate
      if (isCurOpEarly) {
        curOp = inst.job[curOp];
      } else {
        curOp = inst._job[curOp];
      }
    }
  }
  swap_opers(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_insert_earl_late(State &state) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  MoveType bestMoveType, curMoveType;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] != inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  unsigned curOp;
  // check each operation for remove/insertion procedure
  for (unsigned o = 0; o < _ops.size(); ++o) {
    curOp = _ops[o];

    bool isCurOpEarly =
        state.starts[curOp] + inst.P[curOp] < inst.deadlines[curOp];

    // impossible to perform any move
    if (machBlocks[opToBlock[curOp].first].size() == 1)
      continue;

    double curPenal;
    unsigned insertOpCand;
    pair<unsigned, unsigned> move;
    if (isCurOpEarly) {
      insertOpCand = machBlocks[opToBlock[curOp].first].back();
      // if operation has a job successor and the last operation of current
      // block starts after this job successor, search for first operation on
      // block that starts before JS from last until curOp
      if (inst.job[curOp] &&
          state.starts[insertOpCand] > state.starts[inst.job[curOp]]) {
        int j = machBlocks[opToBlock[curOp].first].size() - 1;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[curOp].first][j] != curOp &&
               state.starts[machBlocks[opToBlock[curOp].first][j]] >
                   state.starts[inst.job[curOp]]) {
          insertOpCand = machBlocks[opToBlock[curOp].first][j--];
        }
      }
      move = make_pair(curOp, insertOpCand);
      curMoveType = MoveType::AFTER;
      curPenal = evaluate_insert(state, move, curMoveType);
    } else {
      insertOpCand = machBlocks[opToBlock[curOp].first].front();
      // if operation has a job predecessor and the first operation of current
      // block ends before this job successor, search for first operation on
      // block that starts before JS from first until curOp
      if (inst._job[curOp] &&
          state.starts[insertOpCand] + inst.P[insertOpCand] <
              state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
        int j = 0;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[curOp].first][j] != curOp &&
               state.starts[machBlocks[opToBlock[curOp].first][j]] +
                       inst.P[machBlocks[opToBlock[curOp].first][j]] <
                   state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
          insertOpCand = machBlocks[opToBlock[curOp].first][j++];
        }
      }
      move = make_pair(curOp, insertOpCand);
      curMoveType = MoveType::BEFORE;
      curPenal = evaluate_insert(state, move, curMoveType);
    }

    if (curOp == insertOpCand)
      continue;

    // verify improvement on neighbor and undo remove/insertion
    if (curPenal < bestMovePenalties) {
      bestMove = make_pair(curOp, insertOpCand);
      bestMoveType = curMoveType;
      if (paramTraversing == Parameters::NHoodTraversing::FI &&
          curPenal < initStatePenalties)
        break;
      bestMovePenalties = curPenal;
    }
    assert(state == debugState);
  }
  switch (bestMoveType) {
  case MoveType::AFTER:
    rm_insert_oper_after(state, bestMove.first, bestMove.second);
    break;
  case MoveType::BEFORE:
    rm_insert_oper_befor(state, bestMove.first, bestMove.second);
    break;
  }
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_oper_critical(State &state) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  vector<bool> isOpChecked(inst.O, false);

  unsigned curOp;
  double curPenal;
  vector<pair<unsigned, unsigned>> cands;
  for (unsigned o = 1; o < _ops.size(); ++o) {
    curOp = _ops[o];

    unsigned curBlock;
    unsigned curOpPos;
    while (state._mach[curOp] || inst._job[curOp]) {
      curBlock = opToBlock[curOp].first;
      curOpPos = opToBlock[curOp].second;
      if (isOpChecked[machBlocks[curBlock][curOpPos]])
        break;
      if (curOpPos > 0) {
        cands.push_back(make_pair(machBlocks[curBlock][curOpPos],
                                  machBlocks[curBlock][curOpPos - 1]));
        isOpChecked[machBlocks[curBlock][curOpPos]] = true;
      }
      if (machBlocks[curBlock].size() > 1 &&
          isOpChecked[machBlocks[curBlock][1]])
        break;
      if (curOpPos > 1) {
        cands.push_back(
            make_pair(machBlocks[curBlock][0], machBlocks[curBlock][1]));
        isOpChecked[machBlocks[curBlock][1]] = true;
      }

      for (pair<unsigned, unsigned> cand : cands) {
        curPenal = evaluate_swap(state, cand);
        if (curPenal < bestMovePenalties) {
          bestMove = cand;
          if (paramTraversing == Parameters::NHoodTraversing::FI ||
              curPenal < initStatePenalties) {
            swap_opers(state, bestMove.first, bestMove.second);
            (this->*get_sched_by_param())(state);
            return;
          }
          bestMovePenalties = curPenal;
        }
        assert(debugState == state);
      }

      curOp = inst._job[machBlocks[curBlock][0]];
      cands.clear();
    }
  }
  swap_opers(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}

void Solver::nhood_ls_oper_critical_alt(State &state) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

  // initial state penalties for FI search
  double initStatePenalties = state.penalties;
  // best move found do far. For BI search
  pair<unsigned, unsigned> bestMove(0, 0);
  double bestMovePenalties = DBL_MAX;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  unsigned curOp;
  double curPenal;
  for (unsigned o = 0; o < _ops.size(); ++o) {
    curOp = _ops[o];

    vector<unsigned> opCritic;
    vector<pair<unsigned, unsigned>> cands;

    opCritic.push_back(curOp);
    while (inst._job[curOp] != 0 || state._mach[curOp] != 0) {

      while (state._mach[curOp] &&
             state.starts[state._mach[curOp]] + inst.P[state._mach[curOp]] >
                 state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
        opCritic.push_back(state._mach[curOp]);
        curOp = state._mach[curOp];
      }
      if (opCritic.size() > 1)
        cands.push_back(make_pair(opCritic[0], opCritic[1]));
      if (opCritic.size() > 2)
        cands.push_back(make_pair(opCritic[opCritic.size() - 1],
                                  opCritic[opCritic.size() - 2]));

      for (pair<unsigned, unsigned> cand : cands) {
        curPenal = evaluate_swap(state, cand);
        if (curPenal < bestMovePenalties) {
          bestMove = cand;
          if (paramTraversing == Parameters::NHoodTraversing::FI &&
              curPenal < initStatePenalties) {
            swap_opers(state, bestMove.first, bestMove.second);
            (this->*get_sched_by_param())(state);
            return;
          }
          bestMovePenalties = curPenal;
        }
        assert(debugState == state);
      }

      curOp = inst._job[curOp];
      opCritic.clear();
      cands.clear();
    }
  }
  swap_opers(state, bestMove.first, bestMove.second);
  (this->*get_sched_by_param())(state);
}
