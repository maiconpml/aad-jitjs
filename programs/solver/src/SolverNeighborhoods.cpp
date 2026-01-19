#include "Instance.hpp"
#include "Solver.hpp"
#include <cassert>

void Solver::swap_opers(State &state, const unsigned op1, const unsigned op2) {
#ifndef NDEBUG
  const Instance &inst = Instance::getInstance();
  assert(inst.operToM[op1] == inst.operToJ[op2]);
  assert(op1 < inst.O);
  assert(op2 < inst.O);
#endif // NDEBUG
  assert(op1 * op2);
  if (state.mach[op1] == op2) {
    rm_insert_oper_after(state, op1, op2);
  } else if (state.mach[op2] == op1) {
    rm_insert_oper_after(state, op2, op1);
  } else {
    unsigned _machOp1 = state._mach[op1];
    rm_insert_oper_after(state, op1, op2);
    rm_insert_oper_after(state, op2, _machOp1);
  }
  // TODO: assert if schedule stills valid. There's no scheduler in the project
  // yet.
}

void Solver::rm_insert_oper_after(State &state, const unsigned op1,
                                  const unsigned op2) {
#ifndef NDEBUG
  const Instance &inst = Instance::getInstance();
  assert(inst.operToM[op1] == inst.operToJ[op2]);
  assert(op1 < inst.O);
  assert(op2 < inst.O);
#endif // NDEBUG
  assert(op1);
  unsigned machOp1 = state.mach[op1], _machOp1 = state._mach[op1],
           machOp2 = state.mach[op2];
  if (machOp1)
    state._mach[machOp1] = _machOp1;
  if (_machOp1)
    state.mach[_machOp1] = machOp1;
  if (machOp2)
    state._mach[machOp2] = op1;
  if (op2)
    state.mach[op2] = op1;
  state._mach[op1] = op2;
  state.mach[op1] = machOp2;
  // TODO: assert if schedule stills valid. There's no scheduler in the project
  // yet.
}
