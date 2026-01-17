#pragma once

#include "Instance.hpp"
#include "Parameters.hpp"
#include "State.hpp"

class Solver {
  Solver(const Parameters &parameters);

private:
  // current execution parameters
  Parameters params;
  // current best solution found
  State best;

  // ---------------------------- INITAL SOLUTIONS -----------------------------

  // generate a schedule using Giffler Thompson (1960) algorithm
  void giffler_thompson(State &state) const;
  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state) const;
};
