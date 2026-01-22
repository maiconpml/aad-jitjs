#pragma once

#include <string>
#include <vector>

using namespace std;
class Parameters {
public:
  enum class InitialSolution { GT, CONSTR };

  enum class DispatchingRule { EDD, ACS, RAND };

  enum class Neighborhood {
    SWAP_ADJ,
    SWAP_RAND,
    INSERT_RAND,
    SWAP_PENAL,
    INSERT_PENAL
  };

  enum class NHoodTraversing { BI, FI, ELT_LIST };

  enum class SearchMethod { LS, ILS, TABU };

  enum class Scheduler { EARLY, CPLEX };

  string instPath;
  InitialSolution initialSolution;
  DispatchingRule dispatchingRule;
  vector<Neighborhood> nHoods;
  vector<NHoodTraversing> nHoodsTraversings;
  vector<SearchMethod> searchMethods;
  Scheduler sched;
  unsigned currentSearchMethod;
  unsigned currentNHood;
  unsigned maxMilli;
  unsigned seed;
  bool autoConfig;
};
