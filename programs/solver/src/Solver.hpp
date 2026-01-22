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

  static vector<unsigned> indeg;
  static vector<unsigned> q;

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
  void initial_gt(State &state) const;

  // constructs a solution based on dispatching rule in dispatching. At the end
  // of processing of each operation, uses the dispatching rule to selects next
  // operation for that machine.
  void initial_constr_dispatch(State &state) const;

  // ------------------------------ NEIGHBORHOODS ------------------------------

  // swap operation op1 with operation op2
  void swap_opers(State &state, const unsigned op1, const unsigned op2) const;

  // insert operation op1 after operation op2.
  void rm_insert_oper_after(State &state, const unsigned op1,
                            const unsigned op2) const;

  // insert operation op1 before operation op2.
  void rm_insert_oper_befor(State &state, const unsigned op1,
                            const unsigned op2) const;

  void nhood_swap_adjacent(const State &state, State &neighbor) const;

  void nhood_swap_random(const State &state, State &neighbor) const;

  void nhood_rm_insert_random(const State &state, State &neighbor) const;

  void nhood_swap_earl_late(const State &state, State &neighbor) const;

  void nhood_insert_earl_late(const State &state, State &neighbor) const;

  // ------------------------------ SEARCH METHODS -----------------------------

  void search_ls(State &initialSol);

  // -------------------------------- SCHEDULERS -------------------------------

  // schedule operations as early as possible based on state sequence,
  // return true if state has a cycle
  bool sched_max_early(State &state) const;

  // schedule operations using MIP solver Cplex. Optimal scheduling.
  bool sched_cplex(State &state) const;

  // --------------------------------- EXTRAS ----------------------------------

  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state) const;

  // fills topo with topological sort and returns a boolean indicating if the
  // current graph has one or more cycles
  static bool topo_sort(const State &state);
};
