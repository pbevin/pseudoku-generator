#include <ruby.h>
#include "generate.h"

static void ps_yield(const char *grid, struct stats *stats) {
  rb_yield(rb_str_new2(grid));
}

static VALUE ps_generate() {
  generate(ps_yield);
  return Qnil;
}

void Init_pseudoku() {
  VALUE pseudoku = rb_define_class("Pseudoku", rb_cObject);
  rb_define_method(pseudoku, "generate", ps_generate, 0);
}
