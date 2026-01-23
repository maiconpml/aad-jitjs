#pragma once

#include <boost/multi_array.hpp>

class TabuList {
public:
  TabuList();

  TabuList(unsigned _tenure);

  TabuList(const TabuList &other);

  TabuList &operator=(TabuList other);

  // swaps other with this
  void swap(TabuList &other);

  // move foward in tabu iterations in time iterations
  void passTime(const int time);

  // insert edge o1->o2 in tabu list
  void insert(const int o1, const int o2);

  // is edge o1->o2 tabu?
  bool isTabu(const int o1, const int o2) const;

  // get age of edge o1->o2 in tabu list
  int age(const int o1, const int o2) const;

  // get time to edge o1->o2 leave tabu list
  int timeToLeave(const int o1, const int o2) const;

private:
  // current tabu iteration
  int curIter;
  // number of iterations an move stays tabu
  int tenure;
  // tabu[o1][o2] is iteration wich edge o1->o2 will leave tabu
  boost::multi_array<int, 2> tabu;
};
