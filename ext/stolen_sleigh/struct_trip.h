// ext/stolen_sleigh/struct_trip.h

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definition for Trip and declarations for its memory management
//

#ifndef STRUCT_TRIP_H
#define STRUCT_TRIP_H

// This needs to go at top because utilities.h uses Trip
typedef struct _trip_raw {
  int num_gifts;
  int * gifts;
  double * cumulative_distance;
  double * cumulative_weight;
  double * item_distance; // Distance in previous link *to* gift at this id
  double * item_weights;
  double cost;
  double efficiency;
  } Trip;

#include <ruby.h>
#include "narray.h"
#include "utilities.h"

#define MAX_TRIP_LENGTH 150
#define MAX_TRIP_LENGTH_PLUS_ONE 151

Trip *trip__create();

void trip__init( Trip *trip, int num_gifts );

void trip__init_with_gifts( Trip *trip, int num_gifts, int *new_gifts );

void trip__set_gifts( Trip *trip, VALUE rv_gifts );

void trip__destroy( Trip *trip );

void trip__gc_mark( Trip *trip );

void trip__deep_copy( Trip *trip_copy, Trip *trip_orig );

void trip__quick_copy( Trip *trip_copy, Trip *trip_orig );

Trip * trip__clone( Trip *trip_orig );

double trip__score( Trip *trip );

double trip__weight( Trip *trip );

double trip__distance( Trip *trip );

double trip__efficiency( Trip *trip );

int trip__lowest_cost_insert_at( Trip *trip, int new_gift_id, double * save_cost );

int trip__lowest_cost_break_at( Trip *trip, double * save_cost );

void trip__set_gift( Trip *trip, int idx, int new_gift_id );

void trip__add_gift( Trip *trip, int idx, int new_gift_id );

void trip__remove_gifts_by_idx( Trip *trip, int num, int * idxs );

void trip__remove_gifts_by_gift_id( Trip *trip, int num, int * gift_ids );

int trip__sample_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample );

int trip__sample_gifts_var( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample );

int trip__sample_start_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample );

int trip__sample_end_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample );

void trip__calc_gift_tensions( Trip *trip, double * tensions );

void trip__splice( Trip *trip, int start_idx, int num_splice_out, Trip *input_trip, Trip *output_trip );

void trip__mutual_splice( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a, Trip *trip_b, int idx_out_b, int num_b, int idx_in_b );

void trip__splice_from_a_to_b( Trip *trip_a, int idx_out_a, int num_a, Trip *trip_b, int idx_in_b );

void trip__splice_self( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a );

double trip__mutual_splice_score( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a, Trip *trip_b, int idx_out_b, int num_b, int idx_in_b );

double trip__splice_from_a_to_b_score( Trip *trip_a, int idx_out_a, int num_a, Trip *trip_b, int idx_in_b );

double trip__splice_self_score( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a );

int trip__lowest_cost_insert_at_gdcache( Trip *trip, int new_gift_id, double * save_cost );

void trip__add_gift_gdcache( Trip *trip, int idx, int new_gift_id );

double trip__total_tension( Trip *trip );

#endif
