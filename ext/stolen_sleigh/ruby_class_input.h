// ext/stolen_sleigh/ruby_class_input.h

#ifndef RUBY_CLASS_INPUT_H
#define RUBY_CLASS_INPUT_H

#include <ruby.h>
#include "narray.h"
#include "struct_input.h"
#include "shared_vars.h"

void init_input_class( );

Input *get_input_struct( VALUE obj );

void assert_value_wraps_input( VALUE obj );

#endif
