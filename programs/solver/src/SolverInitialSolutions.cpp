#include "Instance.hpp"
#include "Solver.hpp"

#include <climits>

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

  state.starts.resize(inst.O);
  state.mach.resize(inst.O);
  state._mach.resize(inst.O);

  while (!ready0.empty()) {

    earlComp = UINT_MAX;

    ready1.clear();
    ready2.clear();

    for (unsigned op : ready0) {
      assert(op < inst.O);
      unsigned jobCompTime = state.starts[inst._job[op]] + inst.P[inst._job[op]];
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
      unsigned jobCompTime = state.starts[inst._job[op]] + inst.P[inst._job[op]];
      auxStartTime = max(jobCompTime, machCompTime);
      if (auxStartTime < earlComp) {
        ready2.push_back(op);
      }
    }

    assert(ready2.size() > 0);
    unsigned minDeadline = UINT_MAX;
    unsigned choosedOp;

    for (unsigned op : ready2) {
      if (inst.deadlines[op] < minDeadline) {
        choosedOp = op;
        minDeadline = inst.deadlines[op];
      }
    }

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
