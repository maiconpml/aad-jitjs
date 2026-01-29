#include "Instance.hpp"
#include "Solver.hpp"

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>
#include <queue>

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
  case Parameters::Scheduler::DELAYING:
    return &Solver::sched_delaying;
  }

  return NULL;
}

bool Solver::sched_delaying(State &state) const {
  const Instance &inst = Instance::getInstance();

  unsigned delayTime = 0;
  vector<unsigned> limited(inst.O, 0);
  vector<unsigned> heads;
  vector<unsigned> ops;
  vector<unsigned> lateCands(inst.O, 0);
  vector<unsigned> indeg(inst.O);
  vector<unsigned> Q(inst.O);
  double pushStrength, holdStrength;

  fill(state.starts.begin(), state.starts.end(), 0);

  if (Solver::topo_sort(state))
    return true;
  ops = q;
  reverse(ops.begin(), ops.end());

  for (unsigned i = 0; i < ops.size(); i++) {
    lateCands[ops[i]] = 1;
    heads.push_back(ops[i]);

    if ((inst.job[ops[i]] && (state.starts[inst.job[ops[i]]] < inst.P[ops[i]] ||
                              state.starts[inst.job[ops[i]]] == 0)) ||
        (state.mach[ops[i]] &&
         (state.starts[state.mach[ops[i]]] < inst.P[ops[i]] ||
          state.starts[state.mach[ops[i]]] == 0))) {
      forced_delay(state, lateCands, ops[i]);
    }

    update_strength(lateCands, pushStrength, holdStrength);

    while (pushStrength > holdStrength) {
      delayTime = calc_delay_time(state, lateCands, limited, inst.P);
      delay_opers(state.starts, lateCands, delayTime);
      update_structures(lateCands, limited, heads, state, inst.P);
      update_strength(lateCands, pushStrength, holdStrength);
    }

    fill(lateCands.begin(), lateCands.end(), 0);
    fill(limited.begin(), limited.end(), 0);
    heads.clear();
  }

  state.calc_penalties();

  return false;
}

void Solver::update_strength(vector<unsigned> &lateCands, double &pS,
                             double &hS) const {
  const Instance &inst = Instance::getInstance();
  pS = 0;
  hS = 0;
  for (unsigned i = 1; i < lateCands.size(); i++) {
    if (lateCands[i] == 1)
      pS += inst.earlCoefs[i];
    else if (lateCands[i] == 2)
      hS += inst.tardCoefs[i];
  }
}

unsigned Solver::calc_delay_time(State &state, vector<unsigned> &lateCands,
                                 vector<unsigned> &limited,
                                 const vector<unsigned> &tmpP) const {
  const Instance &inst = Instance::getInstance();
  unsigned t = UINT_MAX;
  unsigned m;
  vector<unsigned> limitedAux;
  for (unsigned i = 1; i < lateCands.size(); i++) {
    if (!lateCands[i])
      continue;
    m = UINT_MAX;
    unsigned aux = state.starts[i] + tmpP[i];
    if (inst.deadlines[i] > aux && inst.deadlines[i] - aux < m)
      m = inst.deadlines[i] - aux;
    if (state.mach[i] &&
        (state.starts[state.mach[i]] > aux || !lateCands[state.mach[i]]) &&
        state.starts[state.mach[i]] - aux < m)
      m = state.starts[state.mach[i]] - aux;
    if (inst.job[i] &&
        (state.starts[inst.job[i]] > aux || !lateCands[inst.job[i]]) &&
        state.starts[inst.job[i]] - aux < m)
      m = state.starts[inst.job[i]] - aux;
    if (m < t) {
      t = m;
      limitedAux.clear();
      limitedAux.push_back(i);
    } else if (m == t)
      limitedAux.push_back(i);
    ;
  }
  fill(limited.begin(), limited.end(), 0);
  for (unsigned i = 0; i < limitedAux.size(); ++i) {
    limited[limitedAux[i]] = 1;
  }
  return t;
}

void Solver::delay_opers(vector<unsigned> &starts, vector<unsigned> &lateCands,
                         unsigned &t) const {
  for (unsigned i = 1; i < lateCands.size(); i++) {
    if (lateCands[i]) {
      starts[i] = starts[i] + t;
    }
  }
}

void Solver::update_structures(vector<unsigned> &lateCands, vector<unsigned> &limited,
                    vector<unsigned> &heads, State &state,
                    const vector<unsigned> &tmpP) const {
  const Instance &inst = Instance::getInstance();
  queue<unsigned> remove;
  queue<unsigned> auxL;
  vector<unsigned> headsCpy(heads);
  vector<unsigned> headsRmv(inst.O, 0);
  unsigned iterations = heads.size();
  heads.clear();
  for (unsigned o = 0; o < iterations; ++o) {
    if (tmpP[headsCpy[o]] + state.starts[headsCpy[o]] ==
        inst.deadlines[headsCpy[o]]) {
      headsRmv[headsCpy[o]] = 1;
      remove.push(o);

      while (!remove.empty()) {
        unsigned c = remove.front();
        remove.pop();

        if (lateCands[state.mach[c]] &&
            tmpP[state.mach[c]] + state.starts[state.mach[c]] >=
                inst.deadlines[state.mach[c]])
          remove.push(state.mach[c]);
        else if (lateCands[state.mach[c]])
          headsCpy.push_back(state.mach[c]);

        if (lateCands[inst.job[c]] &&
            tmpP[inst.job[c]] + state.starts[inst.job[c]] >=
                inst.deadlines[inst.job[c]])
          remove.push(inst.job[c]);
        else if (lateCands[inst.job[c]])
          headsCpy.push_back(inst.job[c]);

        if (limited[c])
          limited[c] = 0;
        lateCands[c] = 0;
      }
    }
  }
  for (unsigned i = 0; i < headsCpy.size(); ++i) {
    if (!headsRmv[headsCpy[i]])
      heads.push_back(headsCpy[i]);
  }

  for (unsigned i = 1; i < inst.O; i++) {
    if (limited[i])
      auxL.push(i);
  }

  while (!auxL.empty()) {
    unsigned o = auxL.front();
    auxL.pop();
    if (tmpP[o] + state.starts[o] == inst.deadlines[o] && lateCands[o] == 1)
      lateCands[o] = 2;
    if (inst.job[o] && tmpP[o] + state.starts[o] == state.starts[inst.job[o]]) {
      auxL.push(inst.job[o]);
      if (tmpP[inst.job[o]] + state.starts[inst.job[o]] >=
          inst.deadlines[inst.job[o]])
        lateCands[inst.job[o]] = 2;
      else
        lateCands[inst.job[o]] = 1;
    }

    if (state.mach[o] &&
        tmpP[o] + state.starts[o] == state.starts[state.mach[o]]) {
      auxL.push(state.mach[o]);
      if (tmpP[state.mach[o]] + state.starts[state.mach[o]] >=
          inst.deadlines[state.mach[o]])
        lateCands[state.mach[o]] = 2;
      else
        lateCands[state.mach[o]] = 1;
    }
  }
}

void Solver::forced_delay(State &state, vector<unsigned> &lateCands,
                          unsigned &op) const {
  const Instance &inst = Instance::getInstance();
  vector<unsigned> auxP = inst.P;
  unsigned co = UINT_MAX;
  unsigned objDelay;
  vector<unsigned> heads;
  vector<unsigned> limited(inst.O, 0);
  if (inst.job[op])
    co = state.starts[inst.job[op]];
  if (state.mach[op] && state.starts[state.mach[op]] < co)
    co = state.starts[state.mach[op]];

  objDelay = auxP[op] - co;
  state.starts[op] = 0;
  auxP[op] = co;
  heads.push_back(op);
  limited[op] = 1;
  update_structures(lateCands, limited, heads, state, auxP);

  while (objDelay > 0) {

    unsigned t = calc_delay_time(state, lateCands, limited, auxP);
    if (objDelay < t)
      t = objDelay;

    delay_opers(state.starts, lateCands, t);
    update_structures(lateCands, limited, heads, state, auxP);
    objDelay -= t;
  }

  state.starts[op] = 0;
}

