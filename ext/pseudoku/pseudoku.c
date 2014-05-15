#include <ruby.h>
#include <ctype.h>
#include "generate.h"
#include "solve.h"

static VALUE ps_alloc(VALUE solverClass) {
  struct solver *solver = malloc(sizeof(struct solver));
  memset(solver, 0, sizeof(*solver));
  initsolve(solver);
  return Data_Wrap_Struct(solverClass, NULL, free, solver);
}

static VALUE ps_solve(VALUE self, VALUE pattern) {
  int puzzle[81];
  char grid[82];
  int i;
  const char *arg = StringValueCStr(pattern);
  struct solver *solver;

  Data_Get_Struct(self, struct solver, solver);

  for (i = 0; i < 81; i++) {
    if (isdigit(arg[i])) {
      puzzle[i] = arg[i] - '0';
    } else {
      puzzle[i] = 0;
    }
  }

  // solve() returns one of:
  //  0: no solution found
  //  1: unique solution found
  //  2: multiple solutions found
  int rc = solve(solver, puzzle);

  if (rc == 1) {
    for (i = 0; i < 81; i++) {
      grid[i] = solver->solution[i] + '0';
    }

    return rb_str_new(grid, 81);
  } else if (rc == 2) {
    return Qfalse;
  } else {
    return Qnil;
  }
}

static VALUE ps_backtracks(VALUE self) {
  struct solver *solver;

  Data_Get_Struct(self, struct solver, solver);
  return INT2NUM(solver->solver_backtracks);
}

static VALUE ps_generate(VALUE self) {
  char grid[82];
  generate(grid);
  return rb_str_new2(grid);
}

void Init_pseudoku() {
  VALUE pseudoku = rb_define_module("Pseudoku");

  VALUE solver = rb_define_class_under(pseudoku, "Solver", rb_cObject);
  rb_define_alloc_func(solver, ps_alloc);
  rb_define_method(solver, "generate", ps_generate, 0);
  rb_define_method(solver, "solve", ps_solve, 1);
  rb_define_method(solver, "backtracks", ps_backtracks, 0);
}
