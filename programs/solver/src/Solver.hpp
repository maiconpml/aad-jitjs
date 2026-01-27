#pragma once

#include "Parameters.hpp"
#include "State.hpp"
#include "TabuList.hpp"
#include <tuple>

class Solver {
public:
  Solver(const Parameters &parameters);

  State solve();

  enum class MoveType { SWAP, BEFORE, AFTER };

private:
  // current execution parameters
  Parameters params;
  // current best solution found
  State best;

  static vector<unsigned> indeg;
  static vector<unsigned> q;
  static vector<unsigned> _ops;

  using SearchPtr = void (Solver::*)(State &);

  using NHoodLSPtr = void (Solver::*)(State &) const;

  using NSTabuPtr = void (Solver::*)(State &, TabuList &) const;

  using CandsPtr = void (Solver::*)(State &) const;

  using SchedPtr = bool (Solver::*)(State &) const;

  static vector<tuple<unsigned, unsigned, MoveType>> _cands;
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

  void cands_swap_adjacent(State &state) const;

  void cands_swap_random(State &state) const;

  void cands_rm_insert_random(State &state) const;

  void cands_swap_earl_late(State &state) const;

  void cands_insert_earl_late(State &state) const;

  void cands_oper_critical(State &state) const;

  void cands_oper_critical_alt(State &state) const;

  void nhood_ls_swap_adjacent(State &state) const;

  void nhood_ls_swap_random(State &state) const;

  void nhood_ls_rm_insert_random(State &state) const;

  void nhood_ls_swap_earl_late(State &state) const;

  void nhood_ls_insert_earl_late(State &state) const;

  void nhood_ls_relax_2(State &state) const;

  void nhood_ls_oper_critical(State &state) const;

  void nhood_ls_oper_critical_alt(State &state) const;
  // ------------------------------- PERTUBATIONS ------------------------------


  void pert_swap_adj_random(State &state, unsigned strength);

  // ------------------------------ SEARCH METHODS -----------------------------

  void search_ils(State &initialSol);

  void search_tabu(State &initialSol);

  void search_ls(State &initialSol);

  // -------------------------------- SCHEDULERS -------------------------------

  // schedule operations as early as possible based on state sequence,
  // return true if state has a cycle
  bool sched_max_early(State &state) const;

  // schedule operations using MIP solver Cplex. Optimal scheduling.
  bool sched_cplex(State &state) const;

  // ---------------------- METHOD GETTERS BY PARAMETERS -----------------------

  SearchPtr get_search_by_param() const;

  SchedPtr get_sched_by_param() const;

  NHoodLSPtr get_nhood_ls_by_param() const;

  NSTabuPtr get_ns_tabu_by_param() const;

  CandsPtr get_cands_by_param() const;
  // --------------------------------- EXTRAS ----------------------------------

  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state) const;

  // fills topo with topological sort and returns a boolean indicating if the
  // current graph has one or more cycles
  static bool topo_sort(const State &state);

  // swap operation op1 with operation op2
  void swap_opers(State &state, const unsigned op1, const unsigned op2) const;

  // insert operation op1 after operation op2.
  void rm_insert_oper_after(State &state, const unsigned op1,
                            const unsigned op2) const;

  // insert operation op1 before operation op2.
  void rm_insert_oper_befor(State &state, const unsigned op1,
                            const unsigned op2) const;

  // return cost of state if swap of operation move.first with operation
  // move.second was applied
  double evaluate_swap(State &state, pair<unsigned, unsigned> &move) const;

  // return cost of state if insertion of operation move.first after (or before
  // depending on type) move.second was applied
  double evaluate_insert(State &state, pair<unsigned, unsigned> &move,
                         Solver::MoveType type) const;

  void update_tabulist_swap(TabuList &tList, State &state,
                            pair<unsigned, unsigned> &move,
                            bool areAllMovesTabu) const;

  void update_tabulist_insert(TabuList &tList, State &state,
                              pair<unsigned, unsigned> &move, MoveType type,
                              bool areAllMovesTabu) const;

  bool is_swap_tabu(TabuList &tList, State &state,
                    pair<unsigned, unsigned> &move) const;

  bool is_insert_tabu(TabuList &tList, State &state,
                      pair<unsigned, unsigned> &move, MoveType type) const;

  int get_swap_tabu_age(TabuList &tList, State &state,
                        pair<unsigned, unsigned> &move) const;

  int get_insert_tabu_age(TabuList &tList, State &state,
                          pair<unsigned, unsigned> &move, MoveType type) const;

  void run_ns_tabu_swap(State &state, TabuList &tList) const;
  void run_ns_tabu_insert(State &state, TabuList &tList) const;
};
