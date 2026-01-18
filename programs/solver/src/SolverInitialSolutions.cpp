#include "Instance.hpp"
#include "Random.hpp"
#include "Solver.hpp"

#include <climits>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

void Solver::giffler_thompson(State &state) const {
  const Instance &inst = Instance::getInstance();

  vector<unsigned> ready0(inst.roots);
  vector<unsigned> ready1;
  vector<unsigned> ready2;
  vector<unsigned> machLeafs(inst.M, 0);

  unsigned compTime;
  unsigned earlComp;
  unsigned selMach = 0;
  unsigned auxStartTime;

  state.starts.resize(inst.O, 0);
  state.mach.resize(inst.O, 0);
  state._mach.resize(inst.O, 0);

  while (!ready0.empty()) {

    earlComp = UINT_MAX;

    ready1.clear();
    ready2.clear();

    for (unsigned op : ready0) {
      assert(op < inst.O);
      unsigned jobCompTime =
          state.starts[inst._job[op]] + inst.P[inst._job[op]];
      unsigned machCompTime = state.starts[machLeafs[inst.operToM[op]]] +
                              inst.P[machLeafs[inst.operToM[op]]];
      compTime = max(jobCompTime, machCompTime) + inst.P[op];
      if (compTime < earlComp) {
        earlComp = compTime;
        selMach = inst.operToM[op];
      }
    }

    for (unsigned op : ready0) {
      if (inst.operToM[op] == selMach) {
        ready1.push_back(op);
      }
    }

    unsigned machCompTime =
        state.starts[machLeafs[selMach]] + inst.P[machLeafs[selMach]];

    for (unsigned op : ready1) {
      unsigned jobCompTime =
          state.starts[inst._job[op]] + inst.P[inst._job[op]];
      auxStartTime = max(jobCompTime, machCompTime);
      if (auxStartTime < earlComp) {
        ready2.push_back(op);
      }
    }

    assert(ready2.size());

    unsigned choosedOpIdx = dispatch_edd(state, ready2, machCompTime);
    unsigned choosedOp = ready2[choosedOpIdx];

    unsigned jobCompTime =
        state.starts[inst._job[choosedOp]] + inst.P[inst._job[choosedOp]];
    state.starts[choosedOp] = max(jobCompTime, machCompTime);

    if (machLeafs[selMach]) {
      state.mach[machLeafs[selMach]] = choosedOp;
      state._mach[choosedOp] = machLeafs[selMach];
    }

    machLeafs[selMach] = choosedOp;

    auto it = find(ready0.begin(), ready0.end(), choosedOp);
    if (it != ready0.end() - 1) {
      *it = ready0.back();
    }
    ready0.pop_back();

    if (inst.job[choosedOp]) {
      ready0.push_back(inst.job[choosedOp]);
    }
  }

  state.calc_penalties();
}

unsigned Solver::dispatch_edd(const State &state, const vector<unsigned> &ops,
                              const unsigned minStartTimeMachine) const {
  assert(ops.size());
  const Instance &inst = Instance::getInstance();
  unsigned bestIdx = 0;
  for (unsigned i = 1; i < ops.size(); ++i) {
    assert(ops[i]);
    if (inst.deadlines[ops[i]] < inst.deadlines[ops[bestIdx]]) {
      bestIdx = i;
    }
  }
  return bestIdx;
}

unsigned Solver::dispatch_all_cr_spt(const State &state,
                                     const vector<unsigned> &ops,
                                     const unsigned minStartTimeMachine) const {
  assert(ops.size());
  assert(ops[0]);
  const Instance &inst = Instance::getInstance();

  // gets first op as initial
  unsigned selOpIdx = 0;
  bool isSelOpEarly =
      (max(minStartTimeMachine, state.starts[inst._job[ops[selOpIdx]]]) <
       inst.deadlines[ops[selOpIdx]]);

  for (unsigned i = 1; i < ops.size(); ++i) {
    assert(ops[i]);
    unsigned startCurOp =
        max(minStartTimeMachine, state.starts[inst._job[ops[i]]]);
    bool replace = false;

    // if selected op is early and current op is tardy, replace selected
    // if selected op and current op are early and selected has lesser earl
    //    ratio, replace selected
    // if selected op and current op are tardy and selected has bigger tard
    //    ratio than current, replace selected
    // if selected op is tardy and current op is early, nothing to do
    if ((isSelOpEarly &&
         (startCurOp >= inst.deadlines[ops[i]] ||
          (double)inst.P[ops[i]] / inst.earlCoefs[ops[i]] >
              (double)inst.P[ops[selOpIdx]] / inst.earlCoefs[ops[selOpIdx]])) ||
        (!isSelOpEarly && startCurOp >= inst.deadlines[ops[i]] &&
         (double)inst.P[ops[i]] / inst.tardCoefs[ops[i]] <
             (double)inst.P[ops[selOpIdx]] / inst.tardCoefs[ops[selOpIdx]])) {
      replace = true;
    }

    if (replace) {
      selOpIdx = i;
      isSelOpEarly = (startCurOp < inst.deadlines[ops[i]]);
    }
  }

  return selOpIdx;
}

unsigned Solver::dispatch_random(const State &state,
                                 const vector<unsigned> &ops,
                                 const unsigned minStartTimeMachine) const {
  assert(ops.size());
  return Random::get((uint32_t)ops.size());
}
