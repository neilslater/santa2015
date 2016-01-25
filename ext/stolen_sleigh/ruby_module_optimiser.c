// ext/stolen_sleigh/ruby_module_optimiser.c

#include "ruby_module_optimiser.h"

void init_module_optimiser() {
  init_module_optimiser_shaker();
  init_module_optimiser_splicer();
  return;
}
