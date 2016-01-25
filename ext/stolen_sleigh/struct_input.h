// ext/stolen_sleigh/struct_input.h

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definition for Input and declarations for its memory management
//

#ifndef STRUCT_INPUT_H
#define STRUCT_INPUT_H

#include <ruby.h>
#include "narray.h"
#include "utilities.h"

typedef struct _input_raw {
  int *gifts_shape;
  volatile VALUE narr_gifts;
  double *gifts;
  char * used;
  } Input;

Input *input__create();

void input__init( Input *input );

void input__init_used( Input *input );

void input__set_gifts( Input *input, VALUE rv_gifts );

void input__destroy( Input *input );

void input__gc_mark( Input *input );

void input__deep_copy( Input *input_copy, Input *input_orig );

Input * input__clone( Input *input_orig );

void input__sort_gifts_by_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids );

void input__sort_gifts_by_long_only_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids );

void input__sort_gifts_by_weighted_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids, double w_hav, double w_long_only, double w_cyl );

int * input__remaining_gifts_by_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts );

int * input__remaining_gifts_within_bounds( Input *input, double lat_min, double lat_max, double lng_centre, double lng_radius, char *exclusion_list, int *num_gifts );

int * input__remaining_gifts_by_long_only_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts );

int * input__remaining_gifts_by_weighted_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts, double w_hav, double w_long_only, double w_cyl );

int input__remaining_most_costly_gift( Input *input );

int input__remaining_heaviest_gift( Input *input );

void input__mark_as_used( Input *input, int num_gifts, int * used_gift_ids );

int input__remaining_furthest_gift( Input *input );


#endif
