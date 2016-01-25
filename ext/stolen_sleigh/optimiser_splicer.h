// ext/stolen_sleigh/optimiser_splicer.h

#ifndef OPTIMISER_SPLICER_H
#define OPTIMISER_SPLICER_H

#include <ruby.h>
#include "narray.h"
#include "shared_vars.h"
#include "shared_helpers.h"
#include "mt.h"
#include "struct_input.h"
#include "struct_solution.h"
#include "struct_trip.h"
#include "struct_trip_collection.h"
#include "utilities.h"

// Note this function is not completed
double optimiser_splicer__multi_trip_splice_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double inv_t, double t_factor );

#endif
