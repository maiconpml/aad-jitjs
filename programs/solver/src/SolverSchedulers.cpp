#include "Instance.hpp"
#include "Solver.hpp"

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

  assert(!q.empty());

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

  state.calc_penalties();

  return head < inst.O - 1;
}
