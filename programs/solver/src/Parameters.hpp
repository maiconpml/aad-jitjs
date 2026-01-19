#pragma once

#include <string>
#include <vector>

using namespace std;
class Parameters {
public:
  enum class InitialSolution { GT, CONSTR };

  enum class DispatchingRule { EDD, ACS, RAND };

  enum class Neighborhood { SWAP_ADJ };

  enum class NHoodTraversing { BI, FI, ELT_LIST };

  enum class SearchMethod { LS, ILS, TABU };

  string instPath;
  InitialSolution initialSolution;
  DispatchingRule dispatchingRule;
  vector<Neighborhood> nHoods;
  vector<NHoodTraversing> nHoodsTraversings;
  vector<SearchMethod> searchMethods;
  unsigned currentSearchMethod;
  unsigned currentNHood;
  unsigned maxMilli;
  unsigned seed;
};
