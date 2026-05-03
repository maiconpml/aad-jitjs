#include "Timer.hpp"
#include "src/Instance.hpp"
#include "src/Parameters.hpp"
#include "src/Random.hpp"
#include "src/Solver.hpp"
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

namespace po = boost::program_options;
using namespace boost;
using namespace std;

int main(int argc, char *argv[]) {

  try {
    // (1) process commandline options
    po::options_description desc("Options");
    desc.add_options()("help", "show help")("instPath", po::value<string>(),
                                            "instance")(
        "maxMilli", po::value<unsigned>()->default_value(60000),
        "maximum time in milliseconds")(
        "seed", po::value<unsigned>()->default_value(13),
        "random number generator seed")(
        "initSol", po::value<string>()->default_value("GT"),
        "initial solution algorithm (GT, CONSTR)")(
        "dispatchRule", po::value<string>()->default_value("EDD"),
        "dispatching rule (EDD, ACS, RAND)")(
        "search1", po::value<string>()->default_value("LS"),
        "main search method (LS, ILS, TABU)")(
        "search2", po::value<string>()->default_value("LS"),
        "internal search method if ILS is chosen on search1 (LS, TABU)")(
        "numNHoods", po::value<unsigned>()->default_value(1),
        "number of neighborhoods")(
        "nhood1", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 1")(
        "nhood2", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 2")(
        "nhood3", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 3")(
        "nhood4", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 4")(
        "nhood5", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 5")(
        "nhood6", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 6")(
        "nhood7", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure 7")(
        "nHoodTravers1", po::value<string>()->default_value("BI"),
        "neighborhood traversing 1 (BI, FI, ELT_LIST)")(
        "nHoodTravers2", po::value<string>()->default_value("BI"),
        "neighborhood traversing 2 (BI, FI, ELT_LIST)")(
        "nHoodTravers3", po::value<string>()->default_value("BI"),
        "neighborhood traversing 3 (BI, FI, ELT_LIST)")(
        "nHoodTravers4", po::value<string>()->default_value("BI"),
        "neighborhood traversing 4 (BI, FI, ELT_LIST)")(
        "nHoodTravers5", po::value<string>()->default_value("BI"),
        "neighborhood traversing 5 (BI, FI, ELT_LIST)")(
        "nHoodTravers6", po::value<string>()->default_value("BI"),
        "neighborhood traversing 6 (BI, FI, ELT_LIST)")(
        "nHoodTravers7", po::value<string>()->default_value("BI"),
        "neighborhood traversing 7 (BI, FI, ELT_LIST)")(
        "sched", po::value<string>()->default_value("EARLY"),
        "scheduler (EARLY, CPLEX, DELAYING, HYB)")(
        "solveExact", po::bool_switch()->default_value(false),
        "solve using CPLEX exact method (overrides other search methods)")(
        "autoConfig", po::bool_switch()->default_value(false),
        "print only the result for automatic configuration")(
        "maxD", po::value<unsigned>()->default_value(5),
        "Maximum size of cycle during tabu search")(
        "maxC", po::value<unsigned>()->default_value(3),
        "Maximum number of repetitions of same cycle during tabu search")(
        "tenure", po::value<unsigned>()->default_value(20),
        "number of tabu iterations a move remains tabu")(
        "jumpListSz", po::value<unsigned>()->default_value(20),
        "number of past elite states to be stored to backjump")(
        "initJumpLimit", po::value<unsigned>()->default_value(10),
        "number of iterations without improvement to trigger a backjump")(
        "decreaseDivisor", po::value<unsigned>()->default_value(2),
        "each jump made reduces the jumpLimit by decreaseDivisor times")(
        "perturbationStrength", po::value<unsigned>()->default_value(30),
        "perturbation strength for ILS (1-100)")(
        "perturbation", po::value<string>()->default_value("RELAX_1"),
        "perturbation type for ILS (SWAP_ADJ, RELAX_1)")(
        "ilsIterMaxMilli", po::value<unsigned>()->default_value(1000000000),
        "maximum time in milliseconds of search iteration on ILS");
    po::positional_options_description pod;
    pod.add("instPath", 1); // instance is positional as well
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(pod).run(),
        vm);
    po::notify(vm);

    // (2) check necessaary options
    if (vm.count("help") || !vm.count("instPath")) {
      cout << "necessary fields not present   --   ";
      cout << desc << endl;
      return 0;
    }

    Parameters param;

    // (3) initialize params
    param.instPath = vm["instPath"].as<string>();
    param.maxMilli = vm["maxMilli"].as<unsigned>();
    param.seed = vm["seed"].as<unsigned>();
    param.autoConfig = vm["autoConfig"].as<bool>();
    param.solveExact = vm["solveExact"].as<bool>();
    param.maxD = vm["maxD"].as<unsigned>();
    param.maxC = vm["maxC"].as<unsigned>();
    param.tenure = vm["tenure"].as<unsigned>();
    param.jumpListSize = vm["jumpListSz"].as<unsigned>();
    param.initialJumpLimit = vm["initJumpLimit"].as<unsigned>();
    param.decreaseDivisor = vm["decreaseDivisor"].as<unsigned>();
    param.perturbationStrength = vm["perturbationStrength"].as<unsigned>();
    param.internalSearchTime = vm["ilsIterMaxMilli"].as<unsigned>();

    string perturbationStr = vm["perturbation"].as<string>();
    if (perturbationStr == "SWAP_ADJ") {
      param.perturbationType = Parameters::PerturbationType::SWAP_ADJ;
    } else if (perturbationStr == "RELAX_1") {
      param.perturbationType = Parameters::PerturbationType::RELAX_1;
    } else {
      throw string("Invalid perturbation type: " + perturbationStr);
    }

    string initSolStr = vm["initSol"].as<string>();
    if (initSolStr == "GT") {
      param.initialSolution = Parameters::InitialSolution::GT;
    } else if (initSolStr == "CONSTR") {
      param.initialSolution = Parameters::InitialSolution::CONSTR;
    } else {
      throw string("Invalid initial solution: " + initSolStr);
    }

    string dispRuleStr = vm["dispatchRule"].as<string>();
    if (dispRuleStr == "EDD") {
      param.dispatchingRule = Parameters::DispatchingRule::EDD;
    } else if (dispRuleStr == "ACS") {
      param.dispatchingRule = Parameters::DispatchingRule::ACS;
    } else if (dispRuleStr == "RAND") {
      param.dispatchingRule = Parameters::DispatchingRule::RAND;
    } else {
      throw string("Invalid dispatching rule: " + dispRuleStr);
    }

    string searchMethodStr = vm["search1"].as<string>();
    if (searchMethodStr == "LS") {
      param.searchMethods.push_back(Parameters::SearchMethod::LS);
    } else if (searchMethodStr == "ILS") {
      param.searchMethods.push_back(Parameters::SearchMethod::ILS);
    } else if (searchMethodStr == "TABU") {
      param.searchMethods.push_back(Parameters::SearchMethod::TABU);
    } else {
      throw string("Invalid search method: " + searchMethodStr);
    }
    searchMethodStr = vm["search2"].as<string>();
    if (searchMethodStr == "LS") {
      param.searchMethods.push_back(Parameters::SearchMethod::LS);
    } else if (searchMethodStr == "TABU") {
      param.searchMethods.push_back(Parameters::SearchMethod::TABU);
    } else {
      throw string("Invalid search method: " + searchMethodStr);
    }
    param.currentSearchMethod = 0;

    unsigned numNHoods = vm["numNHoods"].as<unsigned>();

    for (unsigned i = 1; i <= numNHoods; ++i) {
      string key = "nhood" + to_string(i);
      string nhoodStr = vm[key].as<string>();
      if (nhoodStr == "SWAP_ADJ") {
        param.nHoods.push_back(Parameters::Neighborhood::SWAP_ADJ);
      } else if (nhoodStr == "SWAP_RAND") {
        param.nHoods.push_back(Parameters::Neighborhood::SWAP_RAND);
      } else if (nhoodStr == "INSERT_RAND") {
        param.nHoods.push_back(Parameters::Neighborhood::INSERT_RAND);
      } else if (nhoodStr == "SWAP_PENAL") {
        param.nHoods.push_back(Parameters::Neighborhood::SWAP_PENAL);
      } else if (nhoodStr == "INSERT_PENAL") {
        param.nHoods.push_back(Parameters::Neighborhood::INSERT_PENAL);
      } else if (nhoodStr == "CRITICAL_OPER") {
        param.nHoods.push_back(Parameters::Neighborhood::CRITICAL_OPER);
      } else if (nhoodStr == "CRITICAL_OPER_ALT") {
        param.nHoods.push_back(Parameters::Neighborhood::CRITICAL_OPER_ALT);
      } else if (nhoodStr == "RELAX_2") {
        param.nHoods.push_back(Parameters::Neighborhood::RELAX_2);
      } else {
        throw string("Invalid neighborhood: " + nhoodStr);
      }
    }
    param.currentNHood = 0;

    for (unsigned i = 1; i <= numNHoods; ++i) {
      string key = "nHoodTravers" + to_string(i);
      string nhoodTravStr = vm[key].as<string>();
      if (nhoodTravStr == "BI") {
        param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::BI);
      } else if (nhoodTravStr == "FI") {
        param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::FI);
      } else if (nhoodTravStr == "ELT_LIST") {
        param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::ELT_LIST);
      } else {
        throw string("Invalid neighborhood traversing: " + nhoodTravStr);
      }
    }

    string sched = vm["sched"].as<string>();
    if (sched == "EARLY") {
      param.sched = Parameters::Scheduler::EARLY;
    } else if (sched == "CPLEX") {
      param.sched = Parameters::Scheduler::CPLEX;
    } else if (sched == "DELAYING") {
      param.sched = Parameters::Scheduler::DELAYING;
    } else if (sched == "HYB") {
      param.sched = Parameters::Scheduler::EARLY;
      param.hyb_sched = true;
    } else {
      throw string("Invalid scheduler: " + sched);
    }

    Random::initialize(param.seed);

    Instance::getInstance().parse(param.instPath);

    Solver solver(param);
    State sol;
    if (param.solveExact) {
      solver.solve_cplex(sol);
    } else {
      sol = solver.solve();
    }

    const Instance &inst = Instance::getInstance();

    if (param.autoConfig) {
      cout << sol.penalties << endl;
    } else {
      if (param.instPath.find("loose") != string::npos)
        cout << "l";
      if (param.instPath.find("tight") != string::npos)
        cout << "t";
      if (param.instPath.find("equal") != string::npos)
        cout << "e ";
      if (param.instPath.find("tard") != string::npos)
        cout << "t ";
      if (param.instPath.find("test1") != string::npos)
        cout << "1 ";
      if (param.instPath.find("test2") != string::npos)
        cout << "2 ";

      cout << left << setw(4) << inst.J << " " << left << setw(4) << inst.M
           << " ";
      cout << left << setw(10) << sol.penalties << left << setw(10)
           << sol.tPenalty << left << setw(10) << sol.ePenalty << left
           << setw(10) << sol.millisecsFound << left << setw(10)
           << Timer::elapsedMs() << endl;
    }

  } catch (const string &e) {
    cout << e << endl;
  }

  return 0;
}
