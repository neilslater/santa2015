// ext/stolen_sleigh/ruby_class_solution.h

#ifndef RUBY_CLASS_SOLUTION_H
#define RUBY_CLASS_SOLUTION_H

#include <ruby.h>
#include "narray.h"
#include "struct_solution.h"
#include "shared_vars.h"
#include "ruby_class_input.h"

void init_solution_class( );

Solution *get_solution_struct( VALUE obj );

void assert_value_wraps_solution( VALUE obj );

#endif
