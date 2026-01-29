#include "Instance.hpp"
#include "Solver.hpp"

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>

void Solver::solve_cplex(State &state) const {
  const Instance &inst = Instance::getInstance();

  IloEnv env;
  try {
    IloModel model(env);

    // Completion times C_i
    IloNumVarArray C(env, inst.O, 0, IloInfinity, ILOFLOAT);

    // Earliness and Tardiness
    IloNumVarArray E(env, inst.O, 0, IloInfinity, ILOFLOAT);
    IloNumVarArray T(env, inst.O, 0, IloInfinity, ILOFLOAT);

    // Sequence variables y_{ij} = 1 if i precedes j on the same machine

    IloInt bigM = 0;
    for (unsigned i = 1; i < inst.O; ++i) {
      bigM += inst.P[i];
    }

    // Objective Function: Minimize sum (alpha_i * E_i + beta_i * T_i)
    IloExpr objExpr(env);
    for (unsigned i = 1; i < inst.O; ++i) {
      objExpr += inst.earlCoefs[i] * E[i] + inst.tardCoefs[i] * T[i];

      // E_i >= d_i - C_i
      model.add(E[i] >= (IloNum)inst.deadlines[i] - C[i]);
      // T_i >= C_i - d_i
      model.add(T[i] >= C[i] - (IloNum)inst.deadlines[i]);

      // C_i >= P_i (implicitly handled by non-negativity start time, but C_i
      // includes P_i)
      model.add(C[i] >= (IloNum)inst.P[i]);
    }
    model.add(IloMinimize(env, objExpr));

    // Constraints

    // 1. Job Precedence
    // If op i is predecessor of op j in the same job
    for (unsigned i = 1; i < inst.O; ++i) {
      unsigned nextOp = inst.job[i];
      if (nextOp != 0) {
        // C_{next} >= C_{curr} + P_{next}
        model.add(C[nextOp] >= C[i] + (IloNum)inst.P[nextOp]);
      }
    }

    // 2. Machine Precedence (Disjunctive Constraints)
    for (unsigned m = 0; m < inst.M; ++m) {
      const auto &ops = inst.machOpers[m];
      for (size_t k = 0; k < ops.size(); ++k) {
        for (size_t l = k + 1; l < ops.size(); ++l) {
          unsigned i = ops[k];
          unsigned j = ops[l];

          // Variable y_ij: 1 if i precedes j, 0 otherwise
          IloBoolVar y(env);

          // C_j >= C_i + P_j - M * (1 - y)
          // if y=1 (i before j): C_j >= C_i + P_j
          // if y=0 (j before i): C_j >= C_i + P_j - M (redundant)
          model.add(C[j] >= C[i] + (IloNum)inst.P[j] - bigM * (1 - y));

          // C_i >= C_j + P_i - M * y
          // if y=1: C_i >= C_j + P_i - M (redundant)
          // if y=0: C_i >= C_j + P_i
          model.add(C[i] >= C[j] + (IloNum)inst.P[i] - bigM * y);
        }
      }
    }

    // Solve
    IloCplex cplex(model);
    cplex.setParam(IloCplex::Param::Threads, 0);
    cplex.setParam(IloCplex::Param::TimeLimit, params.maxMilli / 1000.0);

    if (cplex.solve()) {
      state.penalties = cplex.getObjValue();
      state.starts.clear();
      state.starts.resize(inst.O);

      state.mach.assign(inst.O, 0);
      state._mach.assign(inst.O, 0);

      // Extract start times
      for (unsigned i = 1; i < inst.O; ++i) {
        state.starts[i] = (unsigned)round(cplex.getValue(C[i])) - inst.P[i];
      }

      // Reconstruct machine sequence
      for (unsigned m = 0; m < inst.M; ++m) {
        const auto &ops = inst.machOpers[m];
        vector<pair<unsigned, unsigned>> machineOps;
        for (unsigned op : ops) {
          machineOps.push_back({state.starts[op], op});
        }
        sort(machineOps.begin(), machineOps.end());

        for (size_t k = 0; k < machineOps.size(); ++k) {
          unsigned cur = machineOps[k].second;
          if (k > 0) {
            unsigned prev = machineOps[k - 1].second;
            state._mach[cur] = prev;
            state.mach[prev] = cur;
          }
        }
      }

      state.calc_penalties();
    } else {
      cerr << "CPLEX failed to find a solution." << endl;
    }

  } catch (IloException &e) {
    cerr << "CPLEX Exception: " << e << endl;
  } catch (...) {
    cerr << "Unknown Exception during CPLEX solve" << endl;
  }

  env.end();
}
