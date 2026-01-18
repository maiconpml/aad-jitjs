#pragma once

#include <string>

using namespace std;
class Parameters {
public:
  enum class InitialSolution { GT, CONSTR };

  enum class DispatchingRule { EDD, ACS, RAND };

  string instPath;
  InitialSolution initialSolution;
  DispatchingRule dispatchingRule;
  unsigned maxMilli;
  unsigned seed;
};
