#pragma once

#include <boost/multi_array.hpp>

#include <vector>

using namespace std;
using namespace boost;

class Instance {
private:
  Instance();
  bool init;

public:
  // number of jobs
  unsigned J;
  // number of machines
  unsigned M;
  // (J*M+1) number of operations. Operations are indexed starting on 1.
  // Operation 0 is dummy
  unsigned O;
  // <O> P[o] is the processing time of operation o
  vector<unsigned> P;
  // <Instance.O> job[o] is next operation of operation o in its job
  vector<unsigned> job;
  // <Instance.O> _job[o] is previous operation of operation o in its job
  vector<unsigned> _job;
  // <O> operToJ[o] is the job index of operation o
  vector<unsigned> operToJ;
  // <O> operToM[o] is the machine index of operation o
  vector<unsigned> operToM;
  // <J> roots[j] is first operation of job j
  vector<unsigned> roots;
  // <J> roots[j] is last operation of job j
  vector<unsigned> leafs;
  // <O> deadlines[o] is the due time of operation of operation o
  vector<unsigned> deadlines;
  // <O> earlCoefs[o] is the earliness coefficient of operation o
  vector<double> earlCoefs;
  // <O> tardCoefs[o] is the tardiness coefficient of operation o
  vector<double> tardCoefs;
  // <J, M> jmToOper[j][m] is the operation index of job j in machine m
  multi_array<unsigned, 2> jmToOper;

  static Instance &getInstance();

  void parse(const string &instPath);

  Instance(const Instance &) = delete;
  void operator=(const Instance &) = delete;
};
