// ext/stolen_sleigh/struct_solution.h

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definition for Solution and declarations for its memory management
//

#ifndef STRUCT_SOLUTION_H
#define STRUCT_SOLUTION_H

#include <ruby.h>
#include "narray.h"
#include "utilities.h"
#include "struct_trip.h"

typedef struct _solution_raw {
  int *items_shape;
  volatile VALUE narr_items;
  int32_t *items;
  } Solution;

Solution *solution__create();

void solution__init( Solution *solution );

void solution__set_items( Solution *solution, VALUE rv_items );

void solution__destroy( Solution *solution );

void solution__gc_mark( Solution *solution );

void solution__deep_copy( Solution *solution_copy, Solution *solution_orig );

Solution * solution__clone( Solution *solution_orig );

int solution__has_all_gift_ids( Solution *solution );

int solution__all_trips_under_weight_limit( Solution *solution );

int solution__trip_id_is_ok( Solution *solution, int trip_id, int start_idx );

double solution__score( Solution *solution );

Trip * solution__generate_trip( Solution *solution, int trip_id, int start_idx );

#endif
