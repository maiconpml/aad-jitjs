#pragma once

class InstanceGenerator {
public:
  enum class DD { LOOSE, TIGHT };

  enum class W { EQUAL, TARD };

  InstanceGenerator(const DD _dd, const W _w, const unsigned _n,
                    const unsigned _m);

  void generate();

private:
  const DD dd;
  const W w;
  const unsigned n;
  const unsigned m;
};
