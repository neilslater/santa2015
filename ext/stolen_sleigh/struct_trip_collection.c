// ext/stolen_sleigh/struct_trip_collection.c

#include "struct_trip_collection.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions for TripCollection memory management
//

#define NTRIPS 100000

TripCollection *trip_collection__create() {
  TripCollection *trip_collection;
  trip_collection = xmalloc( sizeof(TripCollection) );
  trip_collection->max_trip_id = 0;
  trip_collection->trips = NULL;
  trip_collection->trip_id_of_gift = NULL;
  trip_collection->trip_index_dirty = 1;
  return trip_collection;
}

void trip_collection__init( TripCollection *trip_collection ) {
  int i;

  trip_collection->trips = ALLOC_N( Trip*, NTRIPS );
  trip_collection->trip_id_of_gift = ALLOC_N( int, NTRIPS );
  trip_collection->trip_index_dirty = 0;
  for( i = 0; i < NTRIPS; i++ ) {
    trip_collection->trips[i] = NULL;
    trip_collection->trip_id_of_gift[i] = -1;
  }
  trip_collection->max_trip_id = 0;

  return;
}

void trip_collection__destroy( TripCollection *trip_collection ) {
  int i;
  for ( i = 0; i < trip_collection->max_trip_id; i++ ) {
    xfree( trip_collection->trips[i] );
  }
  xfree( trip_collection->trips );
  xfree( trip_collection->trip_id_of_gift );
  xfree( trip_collection );
  return;
}

void trip_collection__gc_mark( TripCollection *trip_collection ) {
  return;
}

void trip_collection__deep_copy( TripCollection *trip_collection_copy, TripCollection *trip_collection_orig ) {
  int i;
  trip_collection_copy->max_trip_id = trip_collection_orig->max_trip_id;

  trip_collection_copy->trips = ALLOC_N( Trip*, NTRIPS );
  trip_collection_copy->trip_id_of_gift = ALLOC_N( int, NTRIPS );

  for( i = 0; i < trip_collection_copy->max_trip_id; i++ ) {
    trip_collection_copy->trips[i] = NULL;
    trip_collection_copy->trip_id_of_gift[i] = -1;
    if ( trip_collection_orig->trips[i] ) {
      trip_collection_copy->trips[i] = trip__clone( trip_collection_orig->trips[i] );
    }
  }

  // Make copy index itself
  trip_collection_copy->trip_index_dirty = 1;

  return;
}

TripCollection * trip_collection__clone( TripCollection *trip_collection_orig ) {
  TripCollection * trip_collection_copy = trip_collection__create();
  trip_collection__deep_copy( trip_collection_copy, trip_collection_orig );
  return trip_collection_copy;
}

int trip_collection__num_trips( TripCollection *trip_collection ) {
  int i;
  int t = 0;
  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( trip_collection->trips[i] ) {
      t++;
    }
  }
  return t;
}

// Trip objects should be cloned before or after, if going in/out when exposed to end user!
int trip_collection__set_trip( TripCollection *trip_collection, int id, Trip * trip ) {
  if ( id_out_of_bounds( id ) ) {
    return -1;
  }
  if ( trip_collection->trips[id] ) {
    // Delete existing trip
    trip__destroy( trip_collection->trips[id] );
  }
  trip_collection->trips[id] = trip;
  if ( id >= trip_collection->max_trip_id ) {
    trip_collection->max_trip_id = id + 1;
  }

  trip_collection->trip_index_dirty = 1;

  return id;
}

int trip_collection__add_trip( TripCollection *trip_collection, Trip * trip ) {
  int i;
  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( ! trip_collection->trips[i] ) {
      return trip_collection__set_trip( trip_collection, i, trip );
    }
  }
  return trip_collection__set_trip( trip_collection, trip_collection->max_trip_id, trip );
}

Trip * trip_collection__get_trip( TripCollection *trip_collection, int id ) {
  if ( id_out_of_bounds( id ) ) {
    return NULL;
  }
  return trip_collection->trips[id];
}

int trip_collection__delete_trip( TripCollection *trip_collection, int id ) {
  int i;

  if ( id_out_of_bounds( id ) ) {
    return 0;
  }

  if ( trip_collection->trips[id] ) {
    trip__destroy( trip_collection->trips[id] );
    trip_collection->trips[id] = NULL;
    i = trip_collection->max_trip_id - 1;
    while( i > -1 && ! trip_collection->trips[i] ) {
      i--;
    }
    trip_collection->max_trip_id = i + 1;
    trip_collection->trip_index_dirty = 1;

    return 1;
  }

  return 0;
}

double trip_collection__score( TripCollection *trip_collection ) {
  double score = 0.0;
  int i;
  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( trip_collection->trips[i] ) {
      score += trip__score( trip_collection->trips[i] );
    }
  }
  return score;
}

void trip_collection__copy_to_solution( TripCollection *trip_collection, Solution *solution ) {
  int i, j, k;
  k = 0;
  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( trip_collection->trips[i] ) {
      for ( j = 0; j < trip_collection->trips[i]->num_gifts; j++ ) {
        solution->items[ k * 2  ] = trip_collection->trips[i]->gifts[j];
        solution->items[ k * 2 + 1 ] = i;
        k++;
      }
    }
  }
  return;
}

void trip_collection__init_from_solution( TripCollection *trip_collection, Solution *solution ) {
  int i, trip_id;
  char * converted_trips;
  Trip * trip;
  if ( trip_collection->trips ) {
    xfree( trip_collection->trips );
  }
  trip_collection__init( trip_collection );
  converted_trips = create_id_tracker();

  for ( i = 0 ; i < 100000; i++ ) {
    trip_id = solution->items[ 1 + 2 * i ];
    if ( trip_id < 0 || trip_id > 99999 || converted_trips[trip_id] ) {
      continue;
    }
    converted_trips[trip_id] = 1;
    trip = solution__generate_trip( solution, trip_id, i );
    trip_collection__set_trip( trip_collection, trip_id, trip );
  }

  destroy_id_tracker( converted_trips );

  trip_collection->trip_index_dirty = 1;

  return;
}

void trip_collection__index_gifts_by_trip_id( TripCollection *trip_collection ) {
  if ( ! trip_collection->trip_index_dirty ) {
    return;
  }

  int i,j;
  for( i = 0; i < NTRIPS; i++ ) {
    trip_collection->trip_id_of_gift[i] = -1;
  }
  trip_collection->trip_index_dirty = 0;

  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( trip_collection->trips[i] ) {
      for ( j = 0; j < trip_collection->trips[i]->num_gifts; j++ ) {
        trip_collection->trip_id_of_gift[  trip_collection->trips[i]->gifts[j]  ] = i;
      }
    }
  }

  return;
}

int trip_collection__trip_id_of_gift( TripCollection *trip_collection, int gift_id ) {
  trip_collection__index_gifts_by_trip_id( trip_collection );
  if (gift_id < 0 || gift_id > 99999 ) {
    return -1;
  }
  return trip_collection->trip_id_of_gift[ gift_id ];
}

double trip_collection__insert_gift_at_lowest_cost( TripCollection *trip_collection, int gift_id ) {
  int i, best_p, p;
  Trip *trip;
  int best_trip_id = -1;
  double best_cost = 1000000000.0;
  double weight_to_insert = WEIGHT_OF_GIFT(gift_id);

  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( ( trip = trip_collection->trips[i] ) ) {

      if ( weight_to_insert + trip__weight( trip ) > 1000.0 || trip->num_gifts > 149 ) {
        continue;
      }

      p = trip__lowest_cost_insert_at( trip, gift_id, &best_cost );
      if ( p >= 0 ) {
        best_p = p;
        best_trip_id = i;
      }
    }
  }

  if ( best_trip_id > -1 ) {
    trip_collection->trip_index_dirty = 1;
    trip__add_gift( trip_collection->trips[best_trip_id], best_p, gift_id );
    return best_cost;
  }

  return 0.0;
}

void trip_collection__calc_gift_tensions( TripCollection *trip_collection, double * tensions ) {
  int i;
  for( i = 0; i < trip_collection->max_trip_id; i++ ) {
    if ( trip_collection->trips[i] ) {
      trip__calc_gift_tensions( trip_collection->trips[i], tensions );
    }
  }
  return;
}
