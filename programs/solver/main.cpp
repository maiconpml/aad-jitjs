#include "src/Instance.hpp"
#include "src/Parameters.hpp"
#include "src/Solver.hpp"
#include "src/Random.hpp"
#include <boost/program_options.hpp>
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
        "initialSolution", po::value<string>()->default_value("GT"),
        "initial solution algorithm (GT, CONSTR)")(
        "dispatchingRule", po::value<string>()->default_value("EDD"),
        "dispatching rule (EDD, ACS, RAND)");
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

    string initSolStr = vm["initialSolution"].as<string>();
    if (initSolStr == "GT") {
      param.initialSolution = Parameters::InitialSolution::GT;
    } else if (initSolStr == "CONSTR") {
      param.initialSolution = Parameters::InitialSolution::CONSTR;
    } else {
      throw string("Invalid initial solution: " + initSolStr);
    }

    string dispRuleStr = vm["dispatchingRule"].as<string>();
    if (dispRuleStr == "EDD") {
      param.dispatchingRule = Parameters::DispatchingRule::EDD;
    } else if (dispRuleStr == "ACS") {
      param.dispatchingRule = Parameters::DispatchingRule::ACS;
    } else if (dispRuleStr == "RAND") {
      param.dispatchingRule = Parameters::DispatchingRule::RAND;
    } else {
      throw string("Invalid dispatching rule: " + dispRuleStr);
    }

    Random::initialize(param.seed);

    Instance::getInstance().parse(param.instPath);

    Solver solver(param);
    State sol = solver.solve();

    cout << sol.penalties << endl;

  } catch (const string &e) {
    cout << e << endl;
  }

  return 0;
}
