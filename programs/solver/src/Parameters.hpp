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
    INSERT_PENAL,
    CRITICAL_OPER,
    CRITICAL_OPER_ALT
  };

  enum class NHoodTraversing { BI, FI, ELT_LIST };

  enum class SearchMethod { LS, ILS, TABU };

  enum class Scheduler { EARLY, CPLEX, DELAYING, HYB };

  enum class PerturbationType { SWAP_ADJ, RELAX_1 };

  // instance path
  string instPath;
  // maximum time of execution in milliseconds
  unsigned maxMilli;

  // initial solution method
  InitialSolution initialSolution;
  // dispatching rule to use in initial solution
  DispatchingRule dispatchingRule;
  // list of neighborhoods
  vector<Neighborhood> nHoods;
  // current neighborhood being used
  unsigned currentNHood;
  // type of traversing for each neighborhood
  vector<NHoodTraversing> nHoodsTraversings;

  // list of search methods to be used
  vector<SearchMethod> searchMethods;
  // current search method being used
  unsigned currentSearchMethod;

  // type of perturbation to use in ILS
  PerturbationType perturbationType;

  // type of scheduler to evaluate sequence quality
  Scheduler sched;
  unsigned seed;

  // ---------------------- currentSearchMethod == TABU ------------------------

  // maximum size of repeated sequence during tabu
  // search space exploration
  unsigned maxD;
  // maximum times wich the repeated sequence can repeat
  unsigned maxC;
  // number of tabu iterations a move remains tabu
  unsigned tenure;
  // number of past tabu states to go back when stagnation occurs
  unsigned jumpListSize;

  unsigned initialJumpLimit;

  unsigned decreaseDivisor;

  // ---------------------- currentSearchMethod == ILS -------------------------

  unsigned perturbationStrength;

  unsigned internalSearchTime;

  // ------------------------------ EXTRAS -------------------------------------
  // print only objective value for irace
  bool autoConfig;

  // solve using CPLEX exact method
  bool solveExact = false;

  bool hyb_sched = false;
};
