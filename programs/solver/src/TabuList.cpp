#include "TabuList.hpp"
#include "Instance.hpp"

TabuList::TabuList() {}

TabuList::TabuList(unsigned _tenure) : curIter(0), tenure(_tenure) {
  const Instance &inst = Instance::getInstance();
  if (tenure == 0)
    return;
  tabu.resize(boost::extents[inst.O][inst.O]);
  fill(tabu.data(), tabu.data() + tabu.num_elements(), -1);
}

TabuList::TabuList(const TabuList &other) {
  curIter = other.curIter;
  tenure = other.tenure;
  tabu.resize(boost::extents[other.tabu.shape()[0]][other.tabu.shape()[1]]);
  tabu = other.tabu;
}

TabuList &TabuList::operator=(TabuList other) {
  this->swap(other);
  return *this;
}

void TabuList::swap(TabuList &other) {
  using std::swap;
  swap(curIter, other.curIter);
  swap(tenure, other.tenure);
  tabu.resize(boost::extents[other.tabu.shape()[0]][other.tabu.shape()[1]]);
  swap(tabu, other.tabu);
}

void TabuList::passTime(const int time) {
  assert(time <= tenure);
  curIter += time;
}

void TabuList::insert(const int o1, const int o2) {
  assert(o1 != o2);
  assert(tenure != 0);

  curIter++;
  tabu[o1][o2] = curIter + tenure;
}

bool TabuList::isTabu(const int o1, const int o2) const {
  assert(tenure > 0);
  assert(tabu[o1][o2] <= curIter + tenure);
  return tabu[o1][o2] > curIter;
}

int TabuList::age(const int o1, const int o2) const {
  assert(tabu[o1][o2] <= curIter + tenure);
  return tenure + curIter - tabu[o1][o2];
}

int TabuList::timeToLeave(const int o1, const int o2) const {
  int delta = tabu[o1][o2] - curIter;
  assert(delta > 0);
  assert(delta <= tenure);
  assert(tenure - age(o1, o2) == delta);
  return delta;
}
