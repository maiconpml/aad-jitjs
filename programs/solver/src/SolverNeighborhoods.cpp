#include "Instance.hpp"
#include "Parameters.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <cassert>
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
  if (state.mach[op1] == op2) {
    rm_insert_oper_after(state, op1, op2);
  } else if (state.mach[op2] == op1) {
    rm_insert_oper_after(state, op2, op1);
  } else {
    unsigned _machOp1 = state._mach[op1];
    rm_insert_oper_after(state, op1, op2);
    rm_insert_oper_after(state, op2, _machOp1);
  }
  // TODO: assert if schedule stills valid. There's no scheduler in the project
  // yet.
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
  unsigned machOp1 = state.mach[op1], _machOp1 = state._mach[op1],
           machOp2 = state.mach[op2];
  if (machOp1)
    state._mach[machOp1] = _machOp1;
  if (_machOp1)
    state.mach[_machOp1] = machOp1;
  if (machOp2)
    state._mach[machOp2] = op1;
  if (op2)
    state.mach[op2] = op1;
  state._mach[op1] = op2;
  state.mach[op1] = machOp2;
  // TODO: assert if schedule stills valid. There's no scheduler in the project
  // yet.
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
  unsigned op1, op2, prevOp2;
  while (--nMoves) {
    op1 = Random::get(inst.O);
    op2 = inst.machOpers[inst.operToM[op1]][Random::get(inst.J)];
    prevOp2 = curState._mach[op2];
    rm_insert_oper_after(curState, op2, op1);
    if (!sched_max_early(curState)) {
      if (curState.penalties < state.penalties) {
        neighbor = curState;
        return;
      }
    }
    rm_insert_oper_after(curState, op2, prevOp2);
    assert(curState == state);
  }
}
