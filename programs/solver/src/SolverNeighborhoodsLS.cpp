#include "Instance.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <ilcplex/ilocplex.h>
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
  if (bestMove.first != 0) {
    swap_opers(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
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
  if (bestMove.first != 0) {
    swap_opers(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
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
  if (bestMove.first != 0) {
    rm_insert_oper_after(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
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
  if (bestMove.first != 0) {
    swap_opers(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
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

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] != inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(_machBlocks, _opToBlock);

  unsigned curOp;
  // check each operation for remove/insertion procedure
  for (unsigned o = 0; o < _ops.size(); ++o) {
    curOp = _ops[o];

    bool isCurOpEarly =
        state.starts[curOp] + inst.P[curOp] < inst.deadlines[curOp];

    // impossible to perform any move
    if (_machBlocks[_opToBlock[curOp].first].size() == 1)
      continue;

    double curPenal;
    unsigned insertOpCand;
    pair<unsigned, unsigned> move;
    if (isCurOpEarly) {
      insertOpCand = _machBlocks[_opToBlock[curOp].first].back();
      // if operation has a job successor and the last operation of current
      // block starts after this job successor, search for first operation on
      // block that starts before JS from last until curOp
      if (inst.job[curOp] &&
          state.starts[insertOpCand] > state.starts[inst.job[curOp]]) {
        int j = _machBlocks[_opToBlock[curOp].first].size() - 1;
        // TODO: it's possible to perform binary search
        while (_machBlocks[_opToBlock[curOp].first][j] != curOp &&
               state.starts[_machBlocks[_opToBlock[curOp].first][j]] >
                   state.starts[inst.job[curOp]]) {
          insertOpCand = _machBlocks[_opToBlock[curOp].first][j--];
        }
      }
      move = make_pair(curOp, insertOpCand);
      curMoveType = MoveType::AFTER;
      curPenal = evaluate_insert(state, move, curMoveType);
    } else {
      insertOpCand = _machBlocks[_opToBlock[curOp].first].front();
      // if operation has a job predecessor and the first operation of current
      // block ends before this job successor, search for first operation on
      // block that starts before JS from first until curOp
      if (inst._job[curOp] &&
          state.starts[insertOpCand] + inst.P[insertOpCand] <
              state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
        int j = 0;
        // TODO: it's possible to perform binary search
        while (_machBlocks[_opToBlock[curOp].first][j] != curOp &&
               state.starts[_machBlocks[_opToBlock[curOp].first][j]] +
                       inst.P[_machBlocks[_opToBlock[curOp].first][j]] <
                   state.starts[inst._job[curOp]] + inst.P[inst._job[curOp]]) {
          insertOpCand = _machBlocks[_opToBlock[curOp].first][j++];
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
  if (bestMove.first != 0) {
    switch (bestMoveType) {
    case MoveType::AFTER:
      rm_insert_oper_after(state, bestMove.first, bestMove.second);
      break;
    case MoveType::BEFORE:
      rm_insert_oper_befor(state, bestMove.first, bestMove.second);
      break;
    default:
      break;
    }
    (this->*get_sched_by_param())(state);
  }
}

void Solver::nhood_ls_relax_2(State &state) const {
  const Instance &inst = Instance::getInstance();

  // 1. Generate topological sort
  topo_sort(state);
  // q contains the topological sort of all operations 1..inst.O-1

  // 2. Select relaxation center and radius (RR 1 definition)
  unsigned n = inst.J;
  unsigned m = inst.M;
  unsigned rr;
  if (n == 10) {
    rr = (n * m) / 3;
  } else if (n == 15) {
    rr = (n * m) / 4;
  } else if (n == 20) {
    rr = (n * m) / 5;
  } else {
    rr = (n * m) / 4;
  }
  if (rr == 0 && inst.O > 1)
    rr = 1;

  unsigned centerIdx = Random::get(0u, (unsigned)q.size() - 1);

  int startIdx = (int)centerIdx - rr;
  int endIdx = (int)centerIdx + rr;
  if (startIdx < 0)
    startIdx = 0;
  if (endIdx >= (int)q.size())
    endIdx = (int)q.size() - 1;

  vector<bool> isInR(inst.O, false);
  for (int i = startIdx; i <= endIdx; ++i) {
    isInR[q[i]] = true;
  }

  // 4. Build CPLEX model
  IloEnv env;
  try {
    IloModel model(env);

    // Completion times C_i
    IloNumVarArray C(env, inst.O, 0, IloInfinity, ILOFLOAT);

    // Earliness and Tardiness
    IloNumVarArray E(env, inst.O, 0, IloInfinity, ILOFLOAT);
    IloNumVarArray T(env, inst.O, 0, IloInfinity, ILOFLOAT);

    IloNum maxDeadline = 0;
    IloInt sumP = 0;
    for (unsigned i = 1; i < inst.O; ++i) {
      sumP += inst.P[i];
      if (inst.deadlines[i] > maxDeadline)
        maxDeadline = inst.deadlines[i];
    }
    // O Big-M precisa cobrir o limite máximo do horizonte de tempo
    IloNum bigM = maxDeadline + sumP;

    // Objective Function: Minimize sum (alpha_i * E_i + beta_i * T_i)
    IloExpr objExpr(env);
    for (unsigned i = 1; i < inst.O; ++i) {
      objExpr += inst.earlCoefs[i] * E[i] + inst.tardCoefs[i] * T[i];
      model.add(E[i] >= (IloNum)inst.deadlines[i] - C[i]);
      model.add(T[i] >= C[i] - (IloNum)inst.deadlines[i]);
      model.add(C[i] >= (IloNum)inst.P[i]);
    }
    model.add(IloMinimize(env, objExpr));

    // Constraints
    // 1. Job Precedence
    for (unsigned i = 1; i < inst.O; ++i) {
      unsigned nextOp = inst.job[i];
      if (nextOp != 0) {
        model.add(C[nextOp] >= C[i] + (IloNum)inst.P[nextOp]);
      }
    }

    // 2. Machine Precedence
    for (unsigned m_idx = 0; m_idx < inst.M; ++m_idx) {
      // Extract machine sequence from current state
      vector<unsigned> machineSeq;
      unsigned first = 0;
      for (unsigned op : inst.machOpers[m_idx]) {
        if (!state._mach[op]) {
          first = op;
          break;
        }
      }
      unsigned cur = first;
      while (cur) {
        machineSeq.push_back(cur);
        cur = state.mach[cur];
      }

      // In relax-2, we relax machine precedence for ALL operations in R on ALL
      // machines
      for (size_t i = 0; i < machineSeq.size(); ++i) {
        for (size_t j = i + 1; j < machineSeq.size(); ++j) {
          unsigned op1 = machineSeq[i];
          unsigned op2 = machineSeq[j];

          if (isInR[op1] && isInR[op2]) {
            // Both in R: Disjunctive (relaxed)
            IloBoolVar y(env);
            model.add(C[op2] >= C[op1] + (IloNum)inst.P[op2] - bigM * (1 - y));
            model.add(C[op1] >= C[op2] + (IloNum)inst.P[op1] - bigM * y);
          } else {
            // At least one not in R: Fixed relative order (non-relaxed)
            model.add(C[op2] >= C[op1] + (IloNum)inst.P[op2]);
          }
        }
      }
    }

    // Solve
    IloCplex cplex(model);
    cplex.setParam(IloCplex::Param::Threads, 1);
    cplex.setParam(IloCplex::Param::TimeLimit, 1.0); // 1s limit
    cplex.setOut(env.getNullStream());
    cplex.setWarning(env.getNullStream());

    if (cplex.solve()) {
      for (unsigned i = 1; i < inst.O; ++i) {
        state.starts[i] = (unsigned)round(cplex.getValue(C[i])) - inst.P[i];
      }

      for (unsigned m_idx = 0; m_idx < inst.M; ++m_idx) {
        const auto &ops = inst.machOpers[m_idx];
        vector<pair<unsigned, unsigned>> machineOps;
        for (unsigned op : ops) {
          machineOps.push_back({state.starts[op], op});
        }
        sort(machineOps.begin(), machineOps.end());

        for (unsigned op : ops) {
          state.mach[op] = 0;
          state._mach[op] = 0;
        }

        for (size_t k = 0; k < machineOps.size(); ++k) {
          unsigned curOp = machineOps[k].second;
          if (k > 0) {
            unsigned prevOp = machineOps[k - 1].second;
            state._mach[curOp] = prevOp;
            state.mach[prevOp] = curOp;
          }
        }
      }
      state.calc_penalties();
    }
  } catch (...) {
  }
  env.end();
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

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(_machBlocks, _opToBlock);

  vector<bool> isOpChecked(inst.O, false);

  unsigned curOp;
  double curPenal;
  vector<pair<unsigned, unsigned>> cands;
  for (unsigned o = 0; o < _ops.size(); ++o) {
    curOp = _ops[o];

    unsigned curBlock;
    unsigned curOpPos;
    while (state._mach[curOp] || inst._job[curOp]) {
      curBlock = _opToBlock[curOp].first;
      curOpPos = _opToBlock[curOp].second;
      if (isOpChecked[_machBlocks[curBlock][curOpPos]])
        break;
      if (curOpPos > 0) {
        cands.push_back(make_pair(_machBlocks[curBlock][curOpPos],
                                  _machBlocks[curBlock][curOpPos - 1]));
        isOpChecked[_machBlocks[curBlock][curOpPos]] = true;
      }
      if (_machBlocks[curBlock].size() > 1 &&
          isOpChecked[_machBlocks[curBlock][1]])
        break;
      if (curOpPos > 1) {
        cands.push_back(
            make_pair(_machBlocks[curBlock][0], _machBlocks[curBlock][1]));
        isOpChecked[_machBlocks[curBlock][1]] = true;
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

      curOp = inst._job[_machBlocks[curBlock][0]];
      cands.clear();
    }
  }
  if (bestMove.first != 0) {
    swap_opers(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
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

  _ops.clear();
  for (unsigned o = 1; o < state.mach.size(); ++o) {
    if (state.starts[o] + inst.P[o] > inst.deadlines[o])
      _ops.push_back(o);
  }
  // if in FI, explore the neighbors randomly
  if (paramTraversing == Parameters::NHoodTraversing::FI) {
    shuffle(_ops.begin(), _ops.end(), Random::getEngine());
  }

  state.find_blocks(_machBlocks, _opToBlock);

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
  if (bestMove.first != 0) {
    swap_opers(state, bestMove.first, bestMove.second);
    (this->*get_sched_by_param())(state);
  }
}
