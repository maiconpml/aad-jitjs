#pragma once

#include "Parameters.hpp"
#include "State.hpp"

class Solver {
public:
  Solver(const Parameters &parameters);

  State solve();

private:
  // current execution parameters
  Parameters params;
  // current best solution found
  State best;

  // --------------------------- DISPATCHING RULES -----------------------------

  // returns index of operation with earliest due date in ops
  unsigned dispatch_edd(const State &state, const vector<unsigned> &ops,
                        const unsigned minStartTimeMachine) const;

  // returns index of operation in ops based on all + cr + spt dispatching rule
  unsigned dispatch_all_cr_spt(const State &state, const vector<unsigned> &ops,
                               const unsigned minStartTimeMachine) const;

  // returns random index of operations in ops
  unsigned dispatch_random(const State &state, const vector<unsigned> &ops,
                           const unsigned minStartTimeMachine) const;

  // ---------------------------- INITAL SOLUTIONS -----------------------------

  // generate a schedule using Giffler Thompson (1960) algorithm usin
  // dispatching rule dispatching
  void
  giffler_thompson(State &state,
                   const Parameters::DispatchingRule dispatching_choosed) const;

  // constructs a solution based on dispatching rule in dispatching. At the end
  // of processing of each operation, uses the dispatching rule to selects next
  // operation for that machine.
  void construct_by_dispatch(
      State &state,
      const Parameters::DispatchingRule dispatching_choosed) const;
  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state) const;
};
