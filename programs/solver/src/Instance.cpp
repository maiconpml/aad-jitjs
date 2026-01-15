#include "Instance.hpp"

#include <fstream>
#include <stdexcept>

// reads file instance
void Instance::parse(const string &filePath) {
  P.clear();
  operToJ.clear();
  operToM.clear();
  // jSize.clear();
  // mSize.clear();
  // jobOpers.clear();
  // machOpers.clear();
  deadlines.clear();
  earlCoefs.clear();
  tardCoefs.clear();
  string bufferStr;
  double bufferT;
  vector<pair<unsigned, unsigned>> order;
  unsigned fstM, sndM;
  unsigned fstO, sndO;
  vector<unsigned> fromDummy;
  vector<unsigned> toDummy;

  ifstream streamFromFile;
  streamFromFile.open(filePath);
  if (!streamFromFile.is_open())
    throw invalid_argument("Could not open instance file: " + filePath);

  streamFromFile >> J;

  streamFromFile >> M;

  P.push_back(0);
  deadlines.push_back(0);
  earlCoefs.push_back(0);
  tardCoefs.push_back(0);
  operToJ.push_back(UINT_MAX);
  operToM.push_back(UINT_MAX);

  // initializing
  O = 1;
  // jSize.resize(J, 0);
  // mSize.resize(M, 0);

  jmToOper.resize(extents[J][M]);
  // jobOpers.resize(J);
  // machOpers.resize(M);

  unsigned auxM;

  for (unsigned j = 0; j < J; j++) {
    for (unsigned m = 0; m < M; m++) {
      streamFromFile >> auxM;
      streamFromFile >> bufferT;

      assert(bufferT != 0);

      P.push_back(bufferT);
      assert(P[O] == bufferT);
      jmToOper[j][auxM] = O;
      // jobOpers[j].push_back(O);
      // machOpers[auxM].push_back(O);

      streamFromFile >> bufferT;
      deadlines.push_back(bufferT);
      assert(deadlines[O] == bufferT);
      streamFromFile >> bufferT;
      earlCoefs.push_back(bufferT);
      assert(earlCoefs[O] == bufferT);
      streamFromFile >> bufferT;
      tardCoefs.push_back(bufferT);
      assert(tardCoefs[O] == bufferT);

      O++;
      operToJ.push_back(j);
      operToM.push_back(auxM);
      // jSize[j]++;
      // mSize[auxM]++;
    }
  }

  assert(O <= J * M + 1);
  assert(operToJ.size() == O);
  assert(operToM.size() == O);

  streamFromFile.close();

  assert(!streamFromFile.is_open());

  streamFromFile.open(filePath);

  assert(streamFromFile.is_open());

  // Meta that needs O and P
  // next.resize(O);
  // prev.resize(O);

  streamFromFile >> bufferT;
  streamFromFile >> bufferT;

  // Reading precedences
  for (unsigned j = 0; j < J; j++) {

    order.clear();

    fromDummy.clear();
    toDummy.clear();

    streamFromFile >> fstM;
    streamFromFile >> bufferT;
    streamFromFile >> bufferT;
    streamFromFile >> bufferT;
    streamFromFile >> bufferT;
    for (unsigned u = 1; u < M; u++) {
      assert(fstM < M);
      fstO = jmToOper[j][fstM];
      assert(fstO < O);

      streamFromFile >> sndM;
      assert(sndM < M);
      sndO = jmToOper[j][sndM];
      assert(sndO < O);

      // if (fstO != 0 && sndO != 0) {
      //   next[fstO].push_back(sndO);
      //   prev[sndO].push_back(fstO);
      // }

      order.push_back(pair<unsigned, unsigned>(fstM, sndM));
      fstM = sndM;
      streamFromFile >> bufferT;
      streamFromFile >> bufferT;
      streamFromFile >> bufferT;
      streamFromFile >> bufferT;
    }

    // posets.push_back(Poset(order, M));
  }

  // for (unsigned j = 0; j < J; j++) {
  //   assert(!posets[j].roots.empty());
  //   for (unsigned m : posets[j].roots) {
  //     if (jmToOper[j][m] != 0)
  //       roots.push_back(jmToOper[j][m]);
  //   }
  // }
  // for (unsigned j = 0; j < J; j++) {
  //   assert(!posets[j].leafs.empty());
  //   for (unsigned m : posets[j].leafs) {
  //     if (jmToOper[j][m] != 0)
  //       leafs.push_back(jmToOper[j][m]);
  //   }
  // }

  streamFromFile.close();
}
