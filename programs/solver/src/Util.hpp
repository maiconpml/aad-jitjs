#pragma once

#include <vector>

using namespace std;

namespace Util {

// fills topo with topological sort and returns a boolean indicating if the
// current graph has one or more cycles
bool topo_sort(const vector<unsigned int> &job,
               const vector<unsigned int> &mach,
               const vector<unsigned int> &_job,
               const vector<unsigned int> &_mach, vector<unsigned int> &topo);
} // namespace Util
