#pragma once

#include "pcg_random.hpp"
#include <cstdint>

class Random {
public:
  // Initialize the rng with seed
  static void initialize(uint64_t seed);

  // Returns a random int [0, 2^32 - 1]
  static uint32_t get();

  // Returns a random int bounded [0, bound - 1]
  static uint32_t get(uint32_t bound);

  // Returns a random int lower and upper bounded [min, max]
  static uint32_t get(uint32_t min, uint32_t max);

  // Returns a random double [0, 1)
  static double getDouble();

  // Direct access to engine
  static pcg32 &getEngine();

private:
  static pcg32 rng;
};
