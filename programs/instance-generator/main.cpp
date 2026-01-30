#include "src/InstanceGenerator.hpp"
#include "src/Random.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <ostream>
#include <random>
#include <string>

namespace po = boost::program_options;
using namespace boost;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    // (1) process commandline options
    po::options_description desc("Options");
    desc.add_options()("help", "show help")("j", po::value<unsigned>(),
                                            "number of jobs")(
        "m", po::value<unsigned>(), "number of machines")(
        "dd", po::value<string>(), "due date type (TIGHT, LOOSE)")(
        "w", po::value<string>(), "penalties type (EQUAL, TARD)");
    po::positional_options_description pod;
    pod.add("instPath", 1); // instance is positional as well
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(pod).run(),
        vm);
    po::notify(vm);

    // (2) check necessaary options
    if (vm.count("help") ||
        !(vm.count("j") && vm.count("m") && vm.count("w") && vm.count("dd"))) {
      cout << "necessary fields not present   --   ";
      cout << desc << endl;
      return 0;
    }

    // (3) initialize params
    string _dd = vm["dd"].as<string>();
    string _w = vm["w"].as<string>();
    unsigned j = vm["j"].as<unsigned>();
    unsigned m = vm["m"].as<unsigned>();

    InstanceGenerator::DD dd;
    if (_dd == "LOOSE") {
      dd = InstanceGenerator::DD::LOOSE;
    } else if (_dd == "TIGHT") {
      dd = InstanceGenerator::DD::TIGHT;
    } else {
      throw string("Invalid initial solution: " + _dd);
    }

    InstanceGenerator::W w;
    if (_w == "EQUAL") {
      w = InstanceGenerator::W::EQUAL;
    } else if (_w == "TARD") {
      w = InstanceGenerator::W::TARD;
    } else {
      throw string("Invalid initial solution: " + _dd);
    }

    random_device rd;

    Random::initialize(rd());

    InstanceGenerator gen(dd, w, j, m);

    gen.generate();

  } catch (const string &e) {
    cout << e << endl;
  }
  return 0;
}
