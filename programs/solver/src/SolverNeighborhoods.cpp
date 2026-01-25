#include "Instance.hpp"
#include "Parameters.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdint>
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

void Solver::nhood_swap_adjacent(State &state,
                                 pair<unsigned, unsigned> &chosenMove) const {
double Solver::evaluate_swap(State &state,
                             pair<unsigned, unsigned> &move) const {
  SchedPtr sched = get_sched_by_param();

  unsigned i = move.first, j = move.second;
  double cost = DBL_MAX;
  swap_opers(state, i, j);
  if (!(this->*sched)(state)) {
    cost = state.penalties;
  }
  swap_opers(state, j, i);
  return cost;
}

double Solver::evaluate_insert(State &state, pair<unsigned, unsigned> &move,
                               Solver::InsertType type) const {
  SchedPtr sched = get_sched_by_param();
  unsigned i = move.first, j = move.second;
  unsigned _machOp1 = state._mach[i], machOp1 = state.mach[i];
  double cost = DBL_MAX;

  // do move
  switch (type) {
  case Solver::InsertType::BEFORE:
    rm_insert_oper_befor(state, i, j);
    break;
  case Solver::InsertType::AFTER:
    rm_insert_oper_after(state, i, j);
  }
  // if valid, get cost
  if (!(this->*sched)(state)) {
    cost = state.penalties;
  }
  // undo move
  if (_machOp1)
    rm_insert_oper_after(state, i, _machOp1);
  else
    rm_insert_oper_befor(state, i, machOp1);

  return cost;
}

  Parameters::NHoodTraversing paramTraversing =
      params.nHoodsTraversings[params.currentNHood];

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  switch (paramTraversing) {
  case Parameters::NHoodTraversing::BI: {
    State bestNeighbor;
    unsigned i, j;
    for (unsigned o = 1; o < state.mach.size(); ++o) {
      i = o;
      j = state.mach[o];
      if (j) {
        swap_opers(state, i, j);
        if (!(this->*sched)(state)) {
          if (state.penalties < chosenPenalties) {
            chosenMove = make_pair(i, j);
            chosenPenalties = state.penalties;
          }
        }
        swap_opers(state, j, i);
        assert(state == debugState);
      }
    }
    break;
  }
  case Parameters::NHoodTraversing::FI: {
    chosenPenalties = state.penalties;
    vector<pair<unsigned, unsigned>> neighborhoodMoves;
    neighborhoodMoves.reserve(state.mach.size());
    for (unsigned o = 1; o < state.mach.size(); ++o) {
      if (state.mach[o]) {
        neighborhoodMoves.push_back(make_pair(o, state.mach[o]));
      }
    }
    shuffle(neighborhoodMoves.begin(), neighborhoodMoves.end(),
            Random::getEngine());
    for (pair<unsigned, unsigned> move : neighborhoodMoves) {
      swap_opers(state, move.first, move.second);
      if (!(this->*sched)(state)) {
        assert(validate_state(state));
        if (state.penalties < chosenPenalties) {
          chosenMove = move;
          return;
        }
      }
      swap_opers(state, move.first, move.second);
      assert(state == debugState);
    }
    break;
  }
  case Parameters::NHoodTraversing::ELT_LIST:
    break;
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_swap_random(State &state,
                               pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = state.penalties;
  double initStatePenalties = state.penalties;
  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    swap_opers(state, op1, op2);
    if (!(this->*sched)(state)) {
      assert(validate_state(state));
      if (state.penalties < chosenPenalties) {
        chosenMove = make_pair(op1, op2);
        if (state.penalties < initStatePenalties)
          return;
        chosenPenalties = state.penalties;
      }
    }
    swap_opers(state, op1, op2);
    assert(state == debugState);
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_rm_insert_random(
    State &state, pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  double initStatePenalties = state.penalties;
  unsigned nMoves = 2 * inst.O;
  unsigned op1, op2, machOp1, _machOp1;
  while (--nMoves) {
    op1 = Random::get(1, inst.O - 1);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    _machOp1 = state._mach[op1];
    machOp1 = state.mach[op1];
    rm_insert_oper_after(state, op1, op2);
    if (state.mach[op2] != op1 && !(this->*sched)(state)) {
      assert(validate_state(state));
      if (state.penalties < chosenPenalties) {
        chosenMove = make_pair(op1, op2);
        if (state.penalties < initStatePenalties)
          return;
        chosenPenalties = state.penalties;
      }
    }
    if (_machOp1)
      rm_insert_oper_after(state, op1, _machOp1);
    else
      rm_insert_oper_befor(state, op1, machOp1);
    assert(state == debugState);
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_swap_earl_late(State &state,
                                  pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  double initStatePenalties = state.penalties;
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
        swap_opers(state, curOp, swapOpCand);
        if (!(this->*sched)(state)) {
          assert(validate_state(state));
          if (state.penalties < chosenPenalties) {
            chosenMove = make_pair(curOp, swapOpCand);
            if (paramTravers == Parameters::NHoodTraversing::FI &&
                state.penalties < initStatePenalties) {
              return;
            }
            chosenPenalties = state.penalties;
          }
        }
        swap_opers(state, curOp, swapOpCand);
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
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_insert_earl_late(
    State &state, pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  double initStatePenalties = state.penalties;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

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
    if (machBlocks[opToBlock[curOp].first].size() == 1)
      continue;

    unsigned _machCurOp, machCurOp;

    unsigned insertOpCand;
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
      machCurOp = state.mach[curOp];
      _machCurOp = state._mach[curOp];
      rm_insert_oper_after(state, curOp, insertOpCand);
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
      machCurOp = state.mach[curOp];
      _machCurOp = state._mach[curOp];
      rm_insert_oper_befor(state, curOp, insertOpCand);
    }

    if (curOp == insertOpCand)
      continue;

    // verify improvement on neighbor and undo remove/insertion
    if (!(this->*sched)(state)) {
      assert(validate_state(state));
      if (state.penalties < chosenPenalties) {
        chosenMove = make_pair(curOp, insertOpCand);
        if (paramTravers == Parameters::NHoodTraversing::FI &&
            state.penalties < initStatePenalties)
          return;
        chosenPenalties = state.penalties;
      }
    }
    if (_machCurOp)
      rm_insert_oper_after(state, curOp, _machCurOp);
    else
      rm_insert_oper_befor(state, curOp, machCurOp);
    assert(state == debugState);
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_oper_critical(State &state,
                                 pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  double initStatePenalties = state.penalties;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  vector<unsigned> ops;
  ops.reserve(inst.O - 1);

  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      ops.push_back(o);
  }
  if (paramTravers == Parameters::NHoodTraversing::FI) {
    shuffle(ops.begin(), ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  vector<bool> isBlockChecked(machBlocks.size(), false);

  vector<pair<unsigned, unsigned>> cands;
  for (unsigned i = 0; i < ops.size(); ++i) {
    unsigned curOp = ops[i];

    unsigned curBlock;
    unsigned curOpPos;
    while (state._mach[curOp] || inst._job[curOp]) {
      curBlock = opToBlock[curOp].first;
      curOpPos = opToBlock[curOp].second;
      if (curOpPos > 0)
        cands.push_back(make_pair(machBlocks[curBlock][curOpPos],
                                  machBlocks[curBlock][curOpPos - 1]));
      if (curOpPos > 1)
        cands.push_back(
            make_pair(machBlocks[curBlock][0], machBlocks[curBlock][1]));

      for (pair<unsigned, unsigned> cand : cands) {
        swap_opers(state, cand.first, cand.second);
        if (!(this->*sched)(state)) {
          assert(validate_state(state));
          if (state.penalties < chosenPenalties) {
            chosenMove = make_pair(cand.first, cand.second);
            if (paramTravers == Parameters::NHoodTraversing::FI ||
                state.penalties < initStatePenalties)
              return;
            chosenPenalties = state.penalties;
          }
        }
        swap_opers(state, cand.first, cand.second);
        assert(debugState == state);
      }

      curOp = inst._job[machBlocks[curBlock][0]];
      cands.clear();
    }
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}

void Solver::nhood_oper_critical_alt(
    State &state, pair<unsigned, unsigned> &chosenMove) const {
  const Instance &inst = Instance::getInstance();

#ifndef NDEBUG
  State debugState = state;
#endif // NDEBUG

  Parameters::NHoodTraversing paramTravers =
      params.nHoodsTraversings[params.currentNHood];

  SchedPtr sched = get_sched_by_param();

  double chosenPenalties = DBL_MAX;
  double initStatePenalties = state.penalties;
  // machBlocks[b] are operations of block b sorted by start time
  vector<vector<unsigned>> machBlocks;
  // opToBlock[o] is block wich contain operation o
  vector<pair<unsigned, unsigned>> opToBlock(inst.O, make_pair(0, 0));

  vector<unsigned> ops;
  ops.reserve(inst.O - 1);

  for (unsigned o = 1; o < inst.O; ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      ops.push_back(o);
  }
  if (paramTravers == Parameters::NHoodTraversing::FI) {
    shuffle(ops.begin(), ops.end(), Random::getEngine());
  }

  state.find_blocks(machBlocks, opToBlock);

  for (unsigned i = 0; i < ops.size(); ++i) {
    unsigned curOp = ops[i];

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
        swap_opers(state, cand.first, cand.second);
        if (!(this->*sched)(state)) {
          assert(validate_state(state));
          if (state.penalties < chosenPenalties) {
            chosenMove = make_pair(cand.first, cand.second);
            if (paramTravers == Parameters::NHoodTraversing::FI ||
                state.penalties < state.penalties)
              return;
            chosenPenalties = state.penalties;
          }
        }
        swap_opers(state, cand.first, cand.second);
        assert(debugState == state);
      }

      curOp = inst._job[curOp];
      opCritic.clear();
      cands.clear();
    }
  }
  swap_opers(state, chosenMove.first, chosenMove.second);
}
