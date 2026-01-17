#pragma once

#include <string>

enum InitialSolution { GT };

using namespace std;
class Parameters {
public:
  string instPath;
  InitialSolution initialSolution;
  unsigned maxMilli;
  unsigned seed;
};
