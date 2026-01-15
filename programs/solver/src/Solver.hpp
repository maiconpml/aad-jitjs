#pragma once

#include "Instance.hpp"
#include "Parameters.hpp"
#include "State.hpp"

class Solver {
  Solver(const Instance &instance, const Parameters &parameters);

private:
  // current instance being resolved
  Instance inst;
  // current execution parameters
  Parameters params;
  // current best solution found
  State best;
};
