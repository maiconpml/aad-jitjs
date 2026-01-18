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
        "random number generator seed");
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
    param.initialSolution = GT;

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
