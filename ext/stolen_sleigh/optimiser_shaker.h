// ext/stolen_sleigh/optimiser_shaker.h

#ifndef OPTIMISER_SHAKER_H
#define OPTIMISER_SHAKER_H

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

int optimiser_shaker__multi_trip_sample( int num_trips, Trip ** src_trips, int max_sample );
double optimiser_shaker__multi_trip_shake( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws );
double optimiser_shaker__multi_trip_shake_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double start_invt, double t_factor );
double optimiser_shaker__simple_shake_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double inv_t, double t_factor );
void optimiser_shaker__init_buffers();
Trip ** optimiser_shaker__get_tmp_trips();
int * optimiser_shaker__get_gift_pool();

#endif
