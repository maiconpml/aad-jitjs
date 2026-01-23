
#pragma once

#include "TabuTrio.hpp"
class TabuJumpList {

public:
  TabuJumpList(unsigned _maxL);

  ~TabuJumpList();

  // push tt to top of the stack. If stack is full, base element is removed
  void push(const TabuTrio &tt);

  // replace this->cands with cands
  void updateCands(const vector<pair<unsigned, unsigned>> &cands);

  // if stack is not empty, remove top element of stack
  TabuTrio pop();

private:
  // temporal stack of TabuTrios implemented as a circular list
  vector<TabuTrio> ttList;
  // is ttList empty
  bool isEmpty;
  // max elements of ttList
  const unsigned maxL;
  // index of base element of current stack
  unsigned base;
  // top-1 is index of top element of current stack
  unsigned top;
};
