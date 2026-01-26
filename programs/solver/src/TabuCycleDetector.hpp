#pragma once

#include <vector>

using namespace std;

class TabuCycleDetector {
public:
  TabuCycleDetector(unsigned _maxD, unsigned _maxC);

  bool detect(double newObj, bool isNewBest);

private:
  // FIFO queue of last maxD selected objetives implemented as circular list
  vector<double> objQ;
  // position of last inserted element on objQ
  unsigned qFrontPos;
  // (qFrontPos+distFrontCycleLast)%maxD is supposed next objective value in
  // possible current exploring cycle
  unsigned distFrontCycleLast;
  // current size of sequence of repeated objective values
  unsigned cycleSize;
  // max size of sequence of repeated objective values
  unsigned maxD;
  // max times a sequence of repeated objective values can repeat
  unsigned maxC;
};
