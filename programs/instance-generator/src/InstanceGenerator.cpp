#include "InstanceGenerator.hpp"
#include "Random.hpp"
#include <algorithm>
#include <vector>

using namespace std;

InstanceGenerator::InstanceGenerator(const DD _dd, const W _w,
                                     const unsigned _n, const unsigned _m)
    : dd(_dd), w(_w), n(_n), m(_m) {}

void InstanceGenerator::generate() {
  // <i, j> is j-th operation of job i
  vector<vector<unsigned>> p(n, vector<unsigned>(m));
  vector<vector<unsigned>> mach(n, vector<unsigned>(m));
  vector<vector<unsigned>> dueD(n, vector<unsigned>(m));
  vector<vector<double>> earl(n, vector<double>(m));
  vector<vector<double>> tard(n, vector<double>(m));
  unsigned ddOffs = Random::get(1, 25) * 10;

  cout << n << " " << m << endl;
  for (unsigned i = 0; i < n; ++i) {
    unsigned auxDd = ddOffs;
    for (unsigned j = 0; j < m; ++j) {
      // machine initialization
      mach[i][j] = j;

      // processing time definition
      p[i][j] = Random::get(10, 30);

      // due date def
      switch (dd) {
      case DD::TIGHT:
        auxDd += p[i][j];
        dueD[i][j] = auxDd;
        break;
      case DD::LOOSE:
        auxDd += p[i][j];
        dueD[i][j] = auxDd + Random::get(0, 10);
        break;
      }

      // penalties coefs def
      tard[i][j] = (double)Random::get(10, 99) / 100;
      switch (w) {
      case W::EQUAL:
        earl[i][j] = (double)Random::get(10, 99) / 100;
        break;
      case W::TARD:
        earl[i][j] = (double)Random::get(10, 29) / 100;
        break;
      }
    }
    shuffle(mach[i].begin(), mach[i].end(), Random::getEngine());
  }

  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = 0; j < m; ++j) {
      cout << mach[i][j] << " " << p[i][j] << " " << dueD[i][j] << " "
           << earl[i][j] << " " << tard[i][j] << "     ";
    }
    cout << endl;
  }
}
