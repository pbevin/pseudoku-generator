#include <ruby.h>
#include <ctype.h>
#include "generate.h"
#include "solve.h"

static VALUE ps_generate(VALUE self) {
  char grid[82];
  generate(grid);
  return rb_str_new2(grid);
}

static VALUE ps_solve(VALUE self, VALUE pattern) {
  int puzzle[81];
  char grid[82];
  int i;
  const char *arg = StringValueCStr(pattern);

  initsolve();

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
  int rc = solve(puzzle);

  if (rc == 1) {
    for (i = 0; i < 81; i++) {
      grid[i] = solution[i] + '0';
    }

    return rb_str_new(grid, 81);
  } else {
    return Qnil;
  }
}

static VALUE ps_backtracks(VALUE self) {
  return INT2NUM(solver_backtracks);
}

void Init_pseudoku() {
  VALUE pseudoku = rb_define_class("Pseudoku", rb_cObject);
  rb_define_method(pseudoku, "generate", ps_generate, 0);
  rb_define_method(pseudoku, "solve", ps_solve, 1);
  rb_define_method(pseudoku, "backtracks", ps_backtracks, 0);
}
