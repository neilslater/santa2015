// ext/stolen_sleigh/utilities.h

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Declarations of narray helper functions
//

#ifndef CORE_UTILITIES_H
#define CORE_UTILITIES_H

#include <math.h>
#include <immintrin.h>
#include "shared_vars.h"
#include "mt.h"
#include "struct_trip.h"

#define EARTH_RADIUS 6371.0
#define EARTH_DIAMETER 12742.0
#define HALF_PI 1.5707963267948966
#define PI 3.141592653589793
#define TWO_PI 6.283185307179586

int id_out_of_bounds( int id );

double haversine_distance( double lat_a, double long_a, double lat_b, double long_b );

double distance_between_gifts( int gift_a_id, int gift_b_id );

double cartesian_distance_metric( int gift_a_id, int gift_b_id );

double trip_outward_distance( int num_gifts, int * gift_ids );

char * create_id_tracker();

void destroy_id_tracker( char * id_tracker );

void quicksort_ids_by_double( int * ids, double * sort_by, int lowest, int highest );

double trip_cost( int num_gifts, int * gift_ids );

double trip_total_weight( int num_gifts, int * gift_ids );

double trip_separate_cost( int num_gifts, int * gift_ids );

double trip_efficiency( int num_gifts, int * gift_ids );

double trip_distance( int num_gifts, int * gift_ids );

double trip_with_insert_cost( int num_gifts, int * gift_ids, int new_gift_id, int insertion_point );

int trip_lowest_cost_insert( int num_gifts, int * gift_ids, int new_gift_id );

void insert_gift_into_trip_at( int current_trip_size, int * trip_gift_ids, int new_gift_id, int insert_at );

void draw_max_n_from( int num_items, int max_drawn, int * source, int * num_drawn, int * draw_buffer, int * num_remaining, int * remaining_buffer );

void shuffle_ints( int n, int *array );

int trip_lowest_cost_insert2( int num_gifts, int * gift_ids, int new_gift_id, double * save_cost );

double dot_product_sum( int num_items, double * vec_a, double * vec_b );

double fast_dot_product_sum( int num_items, double * vec_a, double * vec_b );

double distance_between_gift_and_longitude( int gift_id, double lng );

double cylinder_distance_between_gift_and_longitude( int gift_id, double lng );

double weighted_metric_distance_between_gifts( int gift_a_id, int gift_b_id, double w_hav, double w_long_only, double w_cyl );

void gdcache__new_cache( int num_gifts, int * gift_ids );

double gdcache__distance_between_gifts( int gift_a_id, int gift_b_id );

void gdcache__new_cache_from_trips( int num_trips, Trip ** trips );

#endif
