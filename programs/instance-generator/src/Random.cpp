#include "Random.hpp"
#include <cmath>

pcg32 Random::rng;

void Random::initialize(uint64_t seed) { rng.seed(seed); }

uint32_t Random::get() { return rng(); }

uint32_t Random::get(uint32_t bound) { return rng(bound); }

uint32_t Random::get(uint32_t min, uint32_t max) {
  if (min > max)
    return get(max, min);
  return min + rng(max - min + 1);
}

double Random::getDouble() { return std::ldexp(rng(), -32); }

pcg32 &Random::getEngine() { return rng; }
