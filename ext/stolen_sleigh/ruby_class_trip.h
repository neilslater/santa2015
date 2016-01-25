// ext/stolen_sleigh/ruby_class_trip.h

#ifndef RUBY_CLASS_TRIP_H
#define RUBY_CLASS_TRIP_H

#include <ruby.h>
#include "narray.h"
#include "struct_trip.h"
#include "shared_vars.h"
#include "ruby_class_input.h"

void init_trip_class( );

Trip *get_trip_struct( VALUE obj );

void assert_value_wraps_trip( VALUE obj );

#endif
