// ext/stolen_sleigh/struct_trip_collection.h

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definition for TripCollection and declarations for its memory management
//

#ifndef STRUCT_TRIP_COLLECTION_H
#define STRUCT_TRIP_COLLECTION_H

#include <ruby.h>
#include "narray.h"
#include "struct_trip.h"
#include "struct_solution.h"

typedef struct _trip_collection_raw {
  int max_trip_id;
  Trip **trips;
  int * trip_id_of_gift;
  int trip_index_dirty;
  } TripCollection;

TripCollection *trip_collection__create();

void trip_collection__init( TripCollection *trip_collection );

void trip_collection__destroy( TripCollection *trip_collection );

void trip_collection__gc_mark( TripCollection *trip_collection );

void trip_collection__deep_copy( TripCollection *trip_collection_copy, TripCollection *trip_collection_orig );

TripCollection * trip_collection__clone( TripCollection *trip_collection_orig );

int trip_collection__num_trips( TripCollection *trip_collection );

int trip_collection__set_trip( TripCollection *trip_collection, int id, Trip * trip );

int trip_collection__add_trip( TripCollection *trip_collection, Trip * trip );

Trip * trip_collection__get_trip( TripCollection *trip_collection, int id );

int trip_collection__delete_trip( TripCollection *trip_collection, int id );

double trip_collection__score( TripCollection *trip_collection );

void trip_collection__copy_to_solution( TripCollection *trip_collection, Solution *solution );

void trip_collection__init_from_solution( TripCollection *trip_collection, Solution *solution );

int trip_collection__trip_id_of_gift( TripCollection *trip_collection, int gift_id );

void trip_collection__index_gifts_by_trip_id( TripCollection *trip_collection );

double trip_collection__insert_gift_at_lowest_cost( TripCollection *trip_collection, int gift_id );

void trip_collection__calc_gift_tensions( TripCollection *trip_collection, double * tensions );

#endif
