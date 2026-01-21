#include "Instance.hpp"
#include "Parameters.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <queue>
#include <utility>

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
  unsigned machOp1 = state.mach[op1], _machOp1 = state._mach[op1],
           _machOp2 = state._mach[op2];
  if (machOp1)
    state._mach[machOp1] = _machOp1;
  if (_machOp1)
    state.mach[_machOp1] = machOp1;
  if (_machOp2)
    state.mach[_machOp2] = op1;
  state._mach[op2] = op1;
  state.mach[op1] = op2;
  state._mach[op1] = _machOp2;
}

void Solver::nhood_swap_adjacent(const State &state, State &neighbor) const {
  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

  State curState = state;
  switch (paramTraversing) {
  case Parameters::NHoodTraversing::BI: {
    State bestNeighbor;
    unsigned i, j;
    for (unsigned o = 1; o < state.mach.size(); ++o) {
      i = o;
      j = curState.mach[o];
      if (j) {
        swap_opers(curState, i, j);
        if (!sched_max_early(curState)) {
          if (curState.penalties < bestNeighbor.penalties) {
            bestNeighbor = curState;
          }
        }
        swap_opers(curState, j, i);
      }
    }
    neighbor = bestNeighbor;
    break;
  }
  case Parameters::NHoodTraversing::FI: {
    vector<pair<unsigned, unsigned>> neighborhoodMoves;
    neighborhoodMoves.reserve(state.mach.size());
    for (unsigned o = 1; o < state.mach.size(); ++o) {
      if (curState.mach[o]) {
        neighborhoodMoves.push_back(make_pair(o, curState.mach[o]));
      }
    }
    shuffle(neighborhoodMoves.begin(), neighborhoodMoves.end(),
            Random::getEngine());
    for (pair<unsigned, unsigned> move : neighborhoodMoves) {
      swap_opers(curState, move.first, move.second);
      if (!sched_max_early(curState)) {
        if (curState.penalties < state.penalties) {
          neighbor = curState;
          return;
        }
      }
      swap_opers(curState, move.first, move.second);
    }
    break;
  }
  case Parameters::NHoodTraversing::ELT_LIST:
    break;
  }
}

void Solver::nhood_swap_random(const State &state, State &neighbor) const {
  const Instance &inst = Instance::getInstance();

  State curState = state;
  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    swap_opers(curState, op1, op2);
    if (!sched_max_early(curState)) {
      if (curState.penalties < state.penalties) {
        neighbor = curState;
        return;
      }
    }
    swap_opers(curState, op1, op2);
    assert(curState == state);
  }
}

void Solver::nhood_rm_insert_random(const State &state, State &neighbor) const {
  const Instance &inst = Instance::getInstance();

  State curState = state;
  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2, machOp1, _machOp1;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    _machOp1 = curState._mach[op1];
    machOp1 = curState.mach[op1];
    rm_insert_oper_after(curState, op1, op2);
    if (curState.mach[op2] != op1 && !sched_max_early(curState)) {
      assert(validate_state(curState));
      if (curState.penalties < state.penalties) {
        neighbor = curState;
        return;
      }
    }
    if (_machOp1)
      rm_insert_oper_after(curState, op1, _machOp1);
    else
      rm_insert_oper_befor(curState, op1, machOp1);
    assert(curState == state);
  }
}

void Solver::nhood_swap_earl_late(const State &state, State &neighbor) const {
  const Instance &inst = Instance::getInstance();

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  State curState = state;
  State bestState;

  // checked[o] == true means that operation o was already verified and if it
  // was possible this swap has already been tested
  vector<bool> checked(inst.O, false);
  // store the operations that not have been picked by the rng, these operations
  // may or may not be checked
  vector<unsigned> notSelected;
  notSelected.reserve(inst.O);

  // operations on time don't need to be verified
  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] == inst.deadlines[o])
      checked[o] = true;
    else
      notSelected.push_back(o);
  }

  while (!notSelected.empty()) {
    // pick an random operation and remove it from notSelected
    unsigned curOpIdx = Random::get((uint32_t)notSelected.size());
    unsigned curOp = notSelected[curOpIdx];
    notSelected[curOpIdx] = notSelected.back();
    notSelected.pop_back();

    bool isCurOpEarly =
        state.starts[curOp] + inst.P[curOp] < inst.deadlines[curOp];

    // while operation exists and is not checked
    while (curOp && !checked[curOp]) {
      checked[curOp] = true;

      // if operation is early (tardy) and it's machine successor (predecessor)
      // exists and is adjecent with curOp, that is, starts (ends) exactly when
      // curOp ends (starts), choose successor (predecessor) as swap candidate
      unsigned swapOpCand = 0;
      if (isCurOpEarly && state.mach[curOp] &&
          state.starts[curOp] + inst.P[curOp] ==
              state.starts[state.mach[curOp]]) {
        swapOpCand = state.mach[curOp];
      } else if (state._mach[curOp] &&
                 state.starts[curOp] == state.starts[state._mach[curOp]] +
                                            inst.P[state._mach[curOp]]) {
        swapOpCand = state._mach[curOp];
      }

      // if swap candidate was selected, execute swap, verify improvement on
      // generated neighbor and undo de swap movement
      if (swapOpCand) {
        swap_opers(curState, curOp, swapOpCand);
        sched_max_early(curState);
        if (curState.penalties < bestState.penalties) {
          if (paramTravers == Parameters::NHoodTraversing::FI &&
              curState.penalties < state.penalties) {
            neighbor = curState;
            return;
          }
          bestState = curState;
        }
        swap_opers(curState, curOp, swapOpCand);
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

  neighbor = bestState;
}

void Solver::nhood_insert_earl_late(const State &state, State &neighbor) const {
  const Instance &inst = Instance::getInstance();

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<unsigned> opToBlock(inst.O, 0);

  State curState = state;
  State bestState;

  vector<unsigned> ops;
  ops.reserve(inst.O - 1);

  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] != inst.deadlines[o])
      ops.push_back(o);
  }
  if (paramTravers == Parameters::NHoodTraversing::FI) {
    shuffle(ops.begin(), ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  // check each operation for remove/insertion procedure
  for (unsigned i = 0; i < ops.size(); ++i) {
    unsigned curOp = ops[i];

    bool isCurOpEarly =
        state.starts[curOp] + inst.P[curOp] < inst.deadlines[curOp];

    // impossible to perform any move
    if (machBlocks[opToBlock[curOp]].size() == 1)
      continue;

    unsigned insertOpCand;
    if (isCurOpEarly) {
      insertOpCand = machBlocks[opToBlock[curOp]].back();
      // if operation has a job successor and the last operation of current
      // block starts after this job successor, search for first operation on
      // block that starts before JS from last until curOp
      if (inst.job[curOp] &&
          state.starts[insertOpCand] > state.starts[inst.job[curOp]]) {
        int j = machBlocks[opToBlock[curOp]].size() - 1;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[curOp]][j] != curOp &&
               state.starts[machBlocks[opToBlock[curOp]][j]] >
                   state.starts[inst.job[curOp]]) {
          insertOpCand = machBlocks[opToBlock[curOp]][j--];
        }
      }
    } else {
      insertOpCand = machBlocks[opToBlock[curOp]].front();
      // if operation has a job predecessor and the first operation of current
      // block ends before this job successor, search for first operation on
      // block that starts before JS from first until curOp
      if (inst._job[curOp] &&
          state.starts[insertOpCand] + inst.P[insertOpCand] <
              state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
        int j = 0;
        // TODO: it's possible to perform binary search
        while (machBlocks[opToBlock[curOp]][j] != curOp &&
               state.starts[machBlocks[opToBlock[curOp]][j]] +
                       inst.P[machBlocks[opToBlock[curOp]][j]] <
                   state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
          insertOpCand = machBlocks[opToBlock[curOp]][j++];
        }
      }
      insertOpCand = state._mach[insertOpCand];
    }

    // neighbor == state
    if (insertOpCand == state._mach[curOp])
      continue;

    // perform remove/insertion, verify improvement on neighbor and undo
    // remove/insertion
    unsigned prevCurOp = inst._job[curOp];
    rm_insert_oper_after(curState, curOp, insertOpCand);
    sched_max_early(curState);
    if (curState.penalties < bestState.penalties) {
      if (paramTravers == Parameters::NHoodTraversing::FI &&
          curState.penalties < state.penalties) {
        neighbor = curState;
        return;
      }
      bestState = curState;
    }
    rm_insert_oper_after(curState, curOp, prevCurOp);
  }

  neighbor = bestState;
}
