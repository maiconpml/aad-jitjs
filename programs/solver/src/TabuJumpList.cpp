#include "TabuJumpList.hpp"

TabuJumpList::TabuJumpList(unsigned _maxL)
    : isEmpty(true), maxL(_maxL), base(0), top(0) {
  ttList.resize(_maxL);
}

TabuJumpList::~TabuJumpList() {}

void TabuJumpList::push(const TabuTrio &tt) {
  assert(maxL > 0);
  if (isEmpty) {
    isEmpty = false;
    top = 0;
    base = 0;
  } else {
    top++;
    top = top % maxL;
    if (top == base) {
      base++;
      base = base % maxL;
    }
  }

  ttList[top] = tt;
}

void TabuJumpList::updateCands(const vector<pair<unsigned, unsigned>> &cands) {
  assert(!isEmpty);

#ifndef NDEBUG
  bool found;
  for (const pair<unsigned, unsigned> &p1 : cands) {
    found = false;
    for (const pair<unsigned, unsigned> &p2 : ttList[top].nhoodMoves) {
      if (p1.first == p2.first && p1.second == p2.second)
        found = true;
    }
    assert(found);
  }
#endif
  ttList[top].nhoodMoves = cands;

  while (ttList[top].nhoodMoves.empty()) {
    if (top == base) {
      isEmpty = true;
      return;
    }

    if (top == 0)
      top = maxL - 1;
    else
      top--;
  }
}

TabuTrio TabuJumpList::pop() {
  if (isEmpty)
    return TabuTrio();

  while (ttList[top].nhoodMoves.empty()) {
    if (top == base) {
      isEmpty = true;
      return TabuTrio();
    }

    if (top == 0)
      top = maxL - 1;
    else
      top--;
  }

  return ttList[top];
}
