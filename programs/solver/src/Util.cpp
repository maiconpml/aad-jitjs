#include "Util.hpp"
#include <cassert>
#include <queue>
#include <vector>

bool Util::topo_sort(const vector<unsigned int> &job,
                     const vector<unsigned int> &mach,
                     const vector<unsigned int> &_job,
                     const vector<unsigned int> &_mach,
                     vector<unsigned int> &topo) {
  assert(job.size() == mach.size());
  assert(mach.size() == _job.size());
  assert(_job.size() == _mach.size());

  topo.clear();

  vector<unsigned> indeg(job.size(), 0);
  queue<unsigned> q;
  int newOp;

  for (unsigned o = 1; o < job.size(); o++) {
    if (_job[o] != 0)
      ++indeg[o];
    if (_mach[o] != 0)
      ++indeg[o];
    if (indeg[o] == 0) {
      q.push(o);
    }
  }
  assert(!q.empty());

  while (!q.empty()) {
    int curOp = q.front();
    q.pop();
    assert(!indeg[curOp]);

    topo.push_back(curOp);

    newOp = job[curOp];
    if (newOp) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp])
        q.push(newOp);
    }

    newOp = mach[curOp];
    if (newOp) {
      assert(indeg[newOp]);
      --indeg[newOp];
      if (!indeg[newOp])
        q.push(newOp);
    }
  }

  assert(topo.size() <= job.size() - 1);

  return topo.size() < job.size() - 1;
}
