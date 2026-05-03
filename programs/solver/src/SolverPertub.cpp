#include "Instance.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <cmath>
#include <ilcplex/ilocplex.h>

void Solver::pert_swap_adj_random(State &state, unsigned strength) {
  const Instance &inst = Instance::getInstance();

  unsigned nSwaps = ceil((float)strength * (float)inst.O / 100);

  while (nSwaps--) {
    unsigned o = Random::get(1, inst.O - 1);

    if (state.mach[o])
      swap_opers(state, o, state.mach[o]);
    if (topo_sort(state)) {
      swap_opers(state, o, state._mach[o]);
    }
  }
}

void Solver::pert_relax_1(State &state, unsigned strength) const {
  const Instance &inst = Instance::getInstance();

  // 1. Generate topological sort
  topo_sort(state);
  // q contains the topological sort of all operations 1..inst.O-1

  // 2. Select relaxation center and radius
  unsigned numJobs = inst.J;
  unsigned numMachines = inst.M;
  unsigned rr;
  if (numJobs == 10) {
    rr = (numJobs * numMachines) / 3;
  } else if (numJobs == 15) {
    rr = (numJobs * numMachines) / 4;
  } else if (numJobs == 20) {
    rr = (numJobs * numMachines) / 5;
  } else {
    // Fallback for other sizes
    rr = (numJobs * numMachines) / 4;
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

  // 3. Select 2 random machines to relax
  unsigned m1 = Random::get(0u, inst.M - 1);
  unsigned m2 = Random::get(0u, inst.M - 1);
  if (inst.M > 1) {
    while (m2 == m1)
      m2 = Random::get(0u, inst.M - 1);
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
    // O tempo de conclusão de qualquer operação no pior cenário nunca passará
    // disso:
    IloNum bigM = maxDeadline + sumP;

    // Objective Function: Minimize sum (alpha_i * E_i + beta_i * T_i)
    IloExpr objExpr(env);
    for (unsigned i = 1; i < inst.O; ++i) {
      objExpr += inst.earlCoefs[i] * E[i] + inst.tardCoefs[i] * T[i];

      // E_i >= d_i - C_i
      model.add(E[i] >= (IloNum)inst.deadlines[i] - C[i]);
      // T_i >= C_i - d_i
      model.add(T[i] >= C[i] - (IloNum)inst.deadlines[i]);

      // C_i >= P_i
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
    for (unsigned m = 0; m < inst.M; ++m) {
      bool isRelaxedMach = (m == m1 || m == m2);

      // Extract machine sequence from current state
      vector<unsigned> machineSeq;
      unsigned first = 0;
      for (unsigned op : inst.machOpers[m]) {
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

      if (!isRelaxedMach) {
        // Entire sequence is fixed
        for (size_t i = 0; i + 1 < machineSeq.size(); ++i) {
          model.add(C[machineSeq[i + 1]] >=
                    C[machineSeq[i]] + (IloNum)inst.P[machineSeq[i + 1]]);
        }
      } else {
        // For relaxed machine, only relax precedences between two ops in R
        for (size_t i = 0; i < machineSeq.size(); ++i) {
          for (size_t j = i + 1; j < machineSeq.size(); ++j) {
            unsigned op1 = machineSeq[i];
            unsigned op2 = machineSeq[j];

            if (isInR[op1] && isInR[op2]) {
              // Both in R on a relaxed machine: Disjunctive
              IloBoolVar y(env);
              model.add(C[op2] >=
                        C[op1] + (IloNum)inst.P[op2] - bigM * (1 - y));
              model.add(C[op1] >= C[op2] + (IloNum)inst.P[op1] - bigM * y);
            } else {
              // At least one not in R: Fixed relative order
              model.add(C[op2] >= C[op1] + (IloNum)inst.P[op2]);
            }
          }
        }
      }
    }

    // Solve
    IloCplex cplex(model);
    cplex.setParam(IloCplex::Param::Threads, 1);
    cplex.setParam(IloCplex::Param::TimeLimit, 1.0); // 1s limit
    cplex.setOut(env.getNullStream());               // Silence CPLEX

    if (cplex.solve()) {
      // Extract start times
      for (unsigned i = 1; i < inst.O; ++i) {
        state.starts[i] = (unsigned)round(cplex.getValue(C[i])) - inst.P[i];
      }

      // Reconstruct machine sequences for all machines based on new start times
      for (unsigned m = 0; m < inst.M; ++m) {
        const auto &ops = inst.machOpers[m];
        vector<pair<unsigned, unsigned>> machineOps;
        for (unsigned op : ops) {
          machineOps.push_back({state.starts[op], op});
        }
        sort(machineOps.begin(), machineOps.end());

        // Clear previous links
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
  } catch (IloException &e) {
    // CPLEX error, state remains unchanged or partially changed
  } catch (...) {
    // Unknown error
  }

  env.end();
}
