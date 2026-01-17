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
  // verify if state satisfy the problem's constraints
  bool validate_state(const State &state);
};
