// ext/stolen_sleigh/struct_solution.c

#include "struct_solution.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions for Solution memory management
//

Solution *solution__create() {
  Solution *solution;
  solution = xmalloc( sizeof(Solution) );
  solution->items_shape = NULL;
  solution->narr_items = Qnil;
  solution->items = NULL;
  return solution;
}

void solution__init( Solution *solution ) {
  int i;
  struct NARRAY *narr;
  int32_t *narr_items_ptr;

  solution->items_shape = ALLOC_N( int, 2 );
  solution->items_shape[0] = 2;
  solution->items_shape[1] = 100000;
  solution->narr_items = na_make_object( NA_LINT, 2, solution->items_shape, cNArray );
  GetNArray( solution->narr_items, narr );
  narr_items_ptr = (int32_t*) narr->ptr;
  for( i = 0; i < narr->total; i++ ) {
    narr_items_ptr[i] = 0;
  }
  solution->items = (int32_t *) narr->ptr;

  return;
}

void solution__set_items( Solution *solution, VALUE rv_items ) {
  int i;
  struct NARRAY *narr;
  double *narr_items_ptr;

  solution->items_shape = ALLOC_N( int, 2 );
  solution->items_shape[0] = 2;
  solution->items_shape[1] = 100000;
  solution->narr_items = rv_items;
  GetNArray( solution->narr_items, narr );
  solution->items = (int32_t *) narr->ptr;

  return;
}

void solution__destroy( Solution *solution ) {
  xfree( solution->items_shape );
  xfree( solution );
  return;
}

void solution__gc_mark( Solution *solution ) {
  rb_gc_mark( solution->narr_items );
  return;
}

void solution__deep_copy( Solution *solution_copy, Solution *solution_orig ) {
  struct NARRAY *narr;
  solution_copy->narr_items = na_clone( solution_orig->narr_items );
  GetNArray( solution_copy->narr_items, narr );
  solution_copy->items = (int32_t *) narr->ptr;
  solution_copy->items_shape = ALLOC_N( int, 2 );
  memcpy( solution_copy->items_shape, solution_orig->items_shape, 2 * sizeof(int) );

  return;
}

Solution * solution__clone( Solution *solution_orig ) {
  Solution * solution_copy = solution__create();
  solution__deep_copy( solution_copy, solution_orig );
  return solution_copy;
}

int solution__has_all_gift_ids( Solution *solution ) {
  // 1. Each gift appears once
  int i, gift_id, is_ok = 1;
  int * gift_counts = ALLOC_N( int, 100000 );

  for( i = 0; i < 100000; i++ ) {
    gift_counts[ i ] = 0;
  }

  for( i = 0; i < 100000; i++ ) {
    gift_id = solution->items[ 0 + 2 * i ];
    if ( gift_id >= 0 && gift_id < 100000) {
      gift_counts[ gift_id ] += 1;
    }
  }

  for( i = 0; i < 100000; i++ ) {
    if ( gift_counts[ i ] != 1 ) {
      is_ok = 0;
      break;
    }
  }

  xfree( gift_counts );
  return is_ok;
}

int solution__all_trips_under_weight_limit( Solution *solution ) {
  int i, trip_id;
  char * seen_trip_id = create_id_tracker();

  for ( i = 0 ; i < 100000; i++ ) {
    trip_id = solution->items[ 1 + 2 * i ];
    if ( trip_id < 0 || trip_id > 99999 ) {
      rb_raise( rb_eRuntimeError, "Bad trip_id %d", trip_id );
    }
    if ( seen_trip_id[trip_id] ) {
      continue;
    }
    seen_trip_id[trip_id] = 1;
    if ( solution__trip_id_is_ok( solution, trip_id, i ) ) {
      continue;
    }
    destroy_id_tracker( seen_trip_id );
    return 0;
  }

  destroy_id_tracker( seen_trip_id );
  return 1;
}

double solution__score( Solution *solution ) {
  if ( ! solution__has_all_gift_ids( solution ) ) {
    return 0.0;
  }

  Trip * trip;
  int i, trip_id;
  double total_score = 0.0;
  char * seen_trip_id = create_id_tracker();

  for ( i = 0 ; i < 100000; i++ ) {
    trip_id = solution->items[ 1 + 2 * i ];
    if ( trip_id < 0 || trip_id > 99999 || seen_trip_id[trip_id] ) {
      continue;
    }
    seen_trip_id[trip_id] = 1;
    trip = solution__generate_trip( solution, trip_id, i );

    total_score += trip__score( trip );

    trip__destroy( trip );
  }

  destroy_id_tracker( seen_trip_id );

  return total_score;
}

Trip * solution__generate_trip( Solution *solution, int trip_id, int start_idx ) {
  Trip * trip;
  int i;
  int num_gifts = 0;
  int * gift_ids = ALLOC_N( int, 1000 );
  for( i = start_idx; i < 100000; i++ ) {
    if ( solution->items[ 1 + 2 * i ] == trip_id ) {
      if ( num_gifts > 150 ) {
        xfree( gift_ids );
        rb_raise( rb_eRuntimeError, "Solution trip_id %d has over 150 gifts", trip_id );
      }
      gift_ids[num_gifts] = solution->items[ 2 * i ];
      num_gifts++;
    }
  }

  trip = trip__create();
  trip__init_with_gifts( trip, num_gifts, gift_ids );

  xfree( gift_ids );
  return trip;
}

int solution__trip_id_is_ok( Solution *solution, int trip_id, int start_idx ) {
  int i, gift_id;
  int num_gifts = 0;
  double total_weight = 0.0;

  for( i = start_idx; i < 100000; i++ ) {
    if ( solution->items[ 1 + 2 * i ] == trip_id ) {
      gift_id = solution->items[ 2 * i ];
      total_weight += WEIGHT_OF_GIFT(gift_id);
      num_gifts++;

      if ( total_weight > 1000.0 || num_gifts > 1000 ) {
        return 0;
      }
    }
  }

  return 1;
}
