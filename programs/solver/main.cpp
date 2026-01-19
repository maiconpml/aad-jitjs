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
        "searchMethod", po::value<string>()->default_value("LS"),
        "search method (LS, ILS, TABU)")(
        "nhood", po::value<string>()->default_value("SWAP_ADJ"),
        "neighborhood structure (SWAP_ADJ)")(
        "nHoodTravers", po::value<string>()->default_value("BI"),
        "neighborhood traversing (BI, FI, ELT_LIST)")(
        "autoConfig", po::bool_switch()->default_value(false),
        "print only the result for automatic configuration");
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

    string searchMethodStr = vm["searchMethod"].as<string>();
    if (searchMethodStr == "LS") {
      param.searchMethods.push_back(Parameters::SearchMethod::LS);
    } else if (searchMethodStr == "ILS") {
      param.searchMethods.push_back(Parameters::SearchMethod::ILS);
    } else if (searchMethodStr == "TABU") {
      param.searchMethods.push_back(Parameters::SearchMethod::TABU);
    } else {
      throw string("Invalid search method: " + searchMethodStr);
    }
    param.currentSearchMethod = 0;

    string nhoodStr = vm["nhood"].as<string>();
    if (nhoodStr == "SWAP_ADJ") {
      param.nHoods.push_back(Parameters::Neighborhood::SWAP_ADJ);
    } else {
      throw string("Invalid neighborhood: " + nhoodStr);
    }
    param.currentNHood = 0;

    string nhoodTravStr = vm["nHoodTravers"].as<string>();
    if (nhoodTravStr == "BI") {
      param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::BI);
    } else if (nhoodTravStr == "FI") {
      param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::FI);
    } else if (nhoodTravStr == "ELT_LIST") {
      param.nHoodsTraversings.push_back(Parameters::NHoodTraversing::ELT_LIST);
    } else {
      throw string("Invalid neighborhood traversing: " + nhoodTravStr);
    }

    Random::initialize(param.seed);

    Instance::getInstance().parse(param.instPath);

    Solver solver(param);
    State sol = solver.solve();

    if (param.autoConfig) {
      cout << sol.penalties << endl;
    } else {
      cout << left << setw(10) << sol.penalties << left << setw(10)
           << sol.tPenalty << left << setw(10) << sol.ePenalty << left
           << setw(10) << sol.millisecsFound << left << setw(10)
           << Timer::elapsedMs() << param.instPath << endl;
    }

  } catch (const string &e) {
    cout << e << endl;
  }

  return 0;
}
