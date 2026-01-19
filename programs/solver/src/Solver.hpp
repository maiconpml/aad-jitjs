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
  initial_gt(State &state,
                   const Parameters::DispatchingRule dispatching_choosed) const;

  // constructs a solution based on dispatching rule in dispatching. At the end
  // of processing of each operation, uses the dispatching rule to selects next
  // operation for that machine.
  void initial_constr_dispatch(
      State &state,
      const Parameters::DispatchingRule dispatching_choosed) const;

  // ------------------------------ NEIGHBORHOODS ------------------------------

  // swap operation op1 with operation op2
  void swap_opers(State &state, const unsigned op1, const unsigned op2);

  // insert operation op1 in front of operation op2. If op2 equals 0, op1 will
  // be put at the beginning of the sequance
  void rm_insert_oper_after(State &state, const unsigned op1,
                            const unsigned op2);
  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state) const;
};
