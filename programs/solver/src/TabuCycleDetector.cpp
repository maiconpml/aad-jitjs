#include "TabuCycleDetector.hpp"
#include <cassert>
#include <cstdlib>

TabuCycleDetector::TabuCycleDetector(unsigned _maxD, unsigned _maxC)
    : qFrontPos(0), distFrontCycleLast(0), cycleSize(0), maxD(_maxD),
      maxC(_maxC) {
  objQ.resize(maxD);
}

bool TabuCycleDetector::detect(double newObj, bool isNewBest) {
  // double comparision tolerance
  const double toler = 1e-2;

  // if newObj is global best, reset cycle detection
  if (isNewBest) {
    qFrontPos = 0;
    distFrontCycleLast = 0;
    cycleSize = 0;
    for (unsigned u = 0; u < maxD; u++)
      objQ[u] = -1;
  }

  // add newObj to objQ
  ++qFrontPos;
  qFrontPos = qFrontPos % maxD;
  objQ[qFrontPos] = newObj;

  // if there's a cycle being detected
  if (distFrontCycleLast) {
    // if next obj in supposed cycle is equal to newObj the cycle is increasing
    if (abs(objQ[(qFrontPos + distFrontCycleLast) % maxD] - newObj) < toler) {
      // if cycle reach max allowed size return true
      if (++cycleSize >= maxC * maxD) {
        return true;
      }
      // the cycle is forming but didn't reach the max size yet
      return false;
    }
  }

  // there's no cycle being detected
  // reset cycle attributes and search for potential cycle, that is, a past
  // objective value equals to newObj
  cycleSize = distFrontCycleLast = 0;
  for (unsigned u = 1; u < maxD; u++) {
    // if found a repeated obj, save the distance from qFrontPos
    if (abs(objQ[(qFrontPos + u) % maxD] - newObj) < toler)
      distFrontCycleLast = u;
  }
  return false;
}
