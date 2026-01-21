#pragma once

#include <vector>

using namespace std;

class State {
public:
  State();

  // <Instance.O> mach[o] is next operation of operation o in its machine in
  // current State
  vector<unsigned> mach;
  // <Instance.O> mach[o] is previous operation of operation o in its machine in
  // current State
  vector<unsigned> _mach;
  // <Instance.O> starts[o] is start time of operation o in current State
  vector<unsigned> starts;
  // from the start of execution, milliseconds when this State was found
  double millisecsFound;
  // tardiness penalty in current State
  double tPenalty;
  // earliness penalty in current State
  double ePenalty;
  // ePenalty + tPenalty for simplicity
  double penalties;

  // returns the total penalties of current schedule. Also updates the
  // corresponding attributes with the new penalties values
  double calc_penalties();

  // fills blocks with each block of operations on the same machine on this
  // state. Maps each operation to one block in opToBlock
  void find_blocks(vector<vector<unsigned>> &blocks,
                   vector<unsigned> &opToBlock) const;

  // only compare the machine ordering
  bool operator==(const State &s);
};
