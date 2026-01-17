#include "Instance.hpp"

#include <fstream>
#include <stdexcept>

Instance::Instance() : init(false) {};

Instance &Instance::getInstance() {
  static Instance inst;
  return inst;
}

// reads file instance
void Instance::parse(const string &filePath) {

  if (init)
    return;

  P.clear();
  operToJ.clear();
  operToM.clear();
  deadlines.clear();
  earlCoefs.clear();
  tardCoefs.clear();

  double buffer;
  ifstream stream;

  stream.open(filePath);
  if (!stream.is_open())
    throw invalid_argument("Could not open instance file: " + filePath);

  P.push_back(0);
  deadlines.push_back(0);
  earlCoefs.push_back(0);
  tardCoefs.push_back(0);
  job.push_back(0);
  _job.push_back(0);
  operToJ.push_back(UINT_MAX);
  operToM.push_back(UINT_MAX);
  jmToOper.resize(extents[J][M]);
  O = 1;

  stream >> J;
  stream >> M;

  for (unsigned j = 0; j < J; ++j) {
    _job.push_back(0);
    roots.push_back(O);
    for (unsigned m = 0; m < M; ++m) {
      operToJ.push_back(j);
      stream >> buffer;
      jmToOper[j][buffer] = O;
      operToM.push_back(buffer);
      stream >> buffer;
      assert(buffer != 0);
      P.push_back(buffer);
      assert(P[O] == buffer);
      stream >> buffer;
      deadlines.push_back(buffer);
      assert(deadlines[O] == buffer);
      stream >> buffer;
      earlCoefs.push_back(buffer);
      assert(earlCoefs[O] == buffer);
      stream >> buffer;
      tardCoefs.push_back(buffer);
      assert(tardCoefs[O] == buffer);
      if (m > 0)
        _job.push_back(O - 1);
      ++O;
      if (m < (M - 1))
        job.push_back(O - 1);
    }
    leafs.push_back(O - 1);
    job.push_back(0);
  }

  assert(O <= J * M + 1);
  assert(operToJ.size() == O);
  assert(operToM.size() == O);

  stream.close();

  assert(!stream.is_open());

  init = true;
}
