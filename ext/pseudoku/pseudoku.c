#include <ruby.h>
#include "generate.h"

static VALUE ps_generate() {
  char grid[82];
  generate(grid);
  return rb_str_new2(grid);
}

void Init_pseudoku() {
  VALUE pseudoku = rb_define_class("Pseudoku", rb_cObject);
  rb_define_method(pseudoku, "generate", ps_generate, 0);
}
