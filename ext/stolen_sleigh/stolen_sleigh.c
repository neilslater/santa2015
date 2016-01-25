// ext/stolen_sleigh/stolen_sleigh.c

#include "ruby_module_stolen_sleigh.h"

/*
 *  Naming conventions used in this C code:
 *
 *  File names
 *    ruby_module_<foo>       :  Ruby bindings for module
 *    ruby_class_<bar>        :  Ruby bindings for class Bar
 *    struct_<baz>            :  C structs for Baz, with memory-management and OO-style "methods"
 *
 *  Variable names
 *    Module_Class_TheThing   :  VALUE container for Ruby Class or Module
 *    The_Thing               :  struct type
 *    the_thing               :  pointer to a struct type
 *
 *  Method names
 *    worker__<desc>          :  OO-style code that takes a Worker C struct as first param
 *    worker_rbobject__<desc> :  Ruby-bound method for StolenSleigh::Worker object
 *    worker_rbclass__<desc>  :  Ruby-bound method for StolenSleigh::Worker class
 *
*/

void Init_stolen_sleigh() {
  init_module_stolen_sleigh();
  init_srand_by_time();
}
