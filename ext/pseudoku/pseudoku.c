#include <ruby.h>
#include <ctype.h>
#include "generate.h"
#include "solve.h"

int debug = 0;

static VALUE ps_generate(VALUE self) {
  char grid[82];
  generate(grid);
  return rb_str_new2(grid);
}

struct solver solver;

static VALUE ps_solve(VALUE self, VALUE pattern) {
  int puzzle[81];
  char grid[82];
  int i;
  const char *arg = StringValueCStr(pattern);

  memset(&solver, 0, sizeof(solver));
  initsolve(&solver);

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
  if (debug) printf("backtracks before: %d\n", solver.solver_backtracks);
  int rc = solve(&solver, puzzle);

  if (debug) printf("backtracks after: %d\n", solver.solver_backtracks);

  if (rc == 1) {
    for (i = 0; i < 81; i++) {
      grid[i] = solver.solution[i] + '0';
    }

    return rb_str_new(grid, 81);
  } else if (rc == 2) {
    return Qfalse;
  } else {
    return Qnil;
  }
}

static VALUE ps_debug(VALUE self) {
  debug = 1;
  return self;
}

static VALUE ps_nodebug(VALUE self) {
  debug = 0;
  return self;
}

static VALUE ps_backtracks(VALUE self) {
  return INT2NUM(solver.solver_backtracks);
}

void Init_pseudoku() {
  VALUE pseudoku = rb_define_class("Pseudoku", rb_cObject);
  rb_define_method(pseudoku, "generate", ps_generate, 0);
  rb_define_method(pseudoku, "solve", ps_solve, 1);
  rb_define_method(pseudoku, "backtracks", ps_backtracks, 0);
  rb_define_method(pseudoku, "debug", ps_debug, 0);
  rb_define_method(pseudoku, "nodebug", ps_nodebug, 0);
}
