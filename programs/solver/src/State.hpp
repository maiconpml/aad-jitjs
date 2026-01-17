#pragma once

#include <vector>

using namespace std;

class State {
public:
  // <Instance.O> mach[o] is next operation of operation o in its machine in
  // current State
  vector<unsigned> mach;
  // <Instance.O> mach[o] is previous operation of operation o in its machine in
  // current State
  vector<unsigned> _mach;
  // <Instance.O> starts[o] is start time of operation o in current State
  vector<unsigned> starts;
  // from the start of execution, milliseconds when this State was found
  unsigned millisecsFound;
  // tardiness penalty in current State
  double tPenalty;
  // earliness penalty in current State
  double ePenalty;
  // ePenalty + tPenalty for simplicity
  double penalties;

  // returns the total penalties of current schedule. Also updates the
  // corresponding attributes with the new penalties values
  double calc_penalties();
};
