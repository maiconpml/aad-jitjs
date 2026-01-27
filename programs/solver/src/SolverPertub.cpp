#include "Instance.hpp"
#include "Random.hpp"
#include "Solver.hpp"
#include <cmath>

void Solver::pert_swap_adj_random(State &state, unsigned strength) {
  const Instance &inst = Instance::getInstance();

  unsigned nSwaps = ceil((float)strength * (float)inst.O / 100);

  while (nSwaps--) {
    unsigned o = Random::get(1, inst.O - 1);

    if (state.mach[o])
      swap_opers(state, o, state.mach[o]);
    if (topo_sort(state)) {
      swap_opers(state, o, state._mach[o]);
    }
  }
}
