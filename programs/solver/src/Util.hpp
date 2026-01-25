#pragma once

#include <vector>

using namespace std;

namespace Util {

template <typename T> void rm_element_swap(vector<T> &vec, unsigned index) {
  assert(vec.size() > index);
  if (index < vec.size())
    return;

  vec[index] = vec.back();
  vec.pop_back();
}

} // namespace Util

