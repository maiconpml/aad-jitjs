#include "Instance.hpp"
#include "Solver.hpp"

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>

void Solver::shift_operations(State &state) const {
  const Instance &inst = Instance::getInstance();

  vector<unsigned> newStarts(inst.O, 0);
  vector<unsigned> startJobSuccessor(inst.J, UINT_MAX);
  vector<unsigned> startMachSuccessor(inst.M, UINT_MAX);

  unsigned curOp;
  unsigned newMakes = 0;

  unsigned tail = q.size();
  while (tail > 0) {

    curOp = q[--tail];

    if (!curOp)
      continue;

    unsigned sMS = startMachSuccessor[inst.operToM[curOp]];
    unsigned sJS = startJobSuccessor[inst.operToJ[curOp]];

    unsigned shiftLimit = min(sJS, sMS);

    unsigned shiftTarget = min(inst.deadlines[curOp], shiftLimit);
    shiftTarget = shiftTarget - inst.P[curOp];

    newStarts[curOp] = max(state.starts[curOp], shiftTarget);

    newMakes = max(newStarts[curOp] + inst.P[curOp], newMakes);

    startJobSuccessor[inst.operToJ[curOp]] = newStarts[curOp];
    startMachSuccessor[inst.operToM[curOp]] = newStarts[curOp];
  }

  state.starts = newStarts;
}

bool Solver::sched_max_early(State &state) const {
  const Instance &inst = Instance::getInstance();

  assert(state.starts.size() == inst.O);

  if (indeg.size() < inst.O) {
    indeg.resize(inst.O);
  }
  fill(indeg.begin(), indeg.end(), 0);

  if (q.capacity() < inst.O) {
    q.reserve(inst.O);
  }
  q.clear();

  unsigned curOp;
  unsigned newOp;
  unsigned head = 0;

  unsigned newMax;

  fill(state.starts.begin(), state.starts.end(), 0);

  for (unsigned o = 1; o < inst.O; o++) {
    if (inst._job[o])
      ++indeg[o];
    if (state._mach[o])
      ++indeg[o];
    if (!indeg[o]) {
      q.push_back(o);
    }
  }

  while (head < q.size()) {
    curOp = q[head++];

    newMax = state.starts[curOp] + inst.P[curOp];

    // from JOB
    newOp = inst.job[curOp];
    if (newOp != 0) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp]) {
        q.push_back(newOp);
      }
      if (state.starts[newOp] < newMax) {
        state.starts[newOp] = newMax;
      }
    }

    // from MACH
    newOp = state.mach[curOp];
    if (newOp != 0) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp]) {
        q.push_back(newOp);
      }
      if (state.starts[newOp] < newMax) {
        state.starts[newOp] = newMax;
      }
    }
  }

  if (head == inst.O - 1) {

#ifndef NDEBUG
    state.calc_penalties();
    double testPenalties = state.penalties;
#endif // NDEBUG

    shift_operations(state);

    state.calc_penalties();
    assert(state.penalties <= testPenalties);
    return false;
  }

  return true;
}

bool Solver::sched_cplex(State &state) const {
  const Instance &inst = Instance::getInstance();

  vector<unsigned> auxJobs = inst.roots;
  vector<vector<unsigned>> machOrder(inst.M, vector<unsigned>(0));

  if (topo_sort(state))
    return true;

  state.starts.clear();
  state.starts.push_back(0);

  IloEnv jitEnv;
  IloModel jitModel(jitEnv);

  IloNumVarArray completionTimes(jitEnv, inst.O - 1, 0, IloInfinity, ILOFLOAT);

  // earliness[i] >= deadlines[i] - completionTimes[i]
  // tardiness[i] >= completionTimes[i] - deadlines[i]
  IloNumVarArray earliness(jitEnv, inst.O - 1, 0, IloInfinity, ILOFLOAT);
  IloNumVarArray tardiness(jitEnv, inst.O - 1, 0, IloInfinity, ILOFLOAT);

  try {
    IloExpr objExpr(jitEnv);
    for (unsigned i = 1; i < inst.O; ++i) {
      /* Objective: Minimize sum of weighted earliness and tardiness */
      objExpr += (earliness[i - 1] * inst.earlCoefs[i]) +
                 (tardiness[i - 1] * inst.tardCoefs[i]);

      /* Linearization constraints for Earliness/Tardiness */
      // E_i >= d_i - C_i  <=> E_i + C_i >= d_i
      jitModel.add(earliness[i - 1] + completionTimes[i - 1] >=
                   (IloNum)inst.deadlines[i]);
      // T_i >= C_i - d_i  <=> T_i - C_i >= -d_i
      jitModel.add(tardiness[i - 1] - completionTimes[i - 1] >=
                   -((IloNum)inst.deadlines[i]));

      /* Start time of a job's first operation must be greater than zero*/
      if (!inst._job[i])
        jitModel.add(completionTimes[i - 1] - inst.P[i] >= 0);

      /* Machine precedence constraints*/
      if (state.mach[i]) {
        jitModel.add(completionTimes[i - 1] <=
                     completionTimes[state.mach[i] - 1] -
                         inst.P[state.mach[i]]);
      }

      /* Job precedence constraints*/
      if (inst.job[i]) {
        jitModel.add(completionTimes[i - 1] <=
                     completionTimes[inst.job[i] - 1] - inst.P[inst.job[i]]);
      }
    }

    jitModel.add(IloMinimize(jitEnv, objExpr));

    IloCplex jitCplex(jitEnv);
    // jitCplex.setParam(IloCplex::Param::TimeLimit, 0.5);
    jitCplex.setOut(jitEnv.getNullStream());

    jitCplex.extract(jitModel);

    jitCplex.solve();

    for (unsigned i = 1; i < inst.O; ++i) {
      state.starts.push_back(
          (unsigned)round(jitCplex.getValue(completionTimes[i - 1])) -
          inst.P[i]);
    }

    state.penalties = jitCplex.getObjValue();
  } catch (IloException &ex) {
    cerr << "Error: " << ex << endl;
  } catch (...) {
    cerr << "Error" << endl;
  }

  jitEnv.end();

  state.calc_penalties();

  return false;
}

Solver::SchedPtr Solver::get_sched_by_param() const {

  Parameters::Scheduler paramSched = params.sched;

  switch (paramSched) {
  case Parameters::Scheduler::EARLY:
    return &Solver::sched_max_early;
  case Parameters::Scheduler::CPLEX:
    return &Solver::sched_cplex;
  }
}
