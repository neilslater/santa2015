// ext/stolen_sleigh/struct_trip.c

#include "struct_trip.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions for Trip memory management
//

inline void trip__clear_cache( Trip *trip ) {
  // Use negative values to represent not cached . . .
  trip->cost = -1.0;
  trip->efficiency = -1.0;
  return;
}

inline void trip__recalc_distances( Trip *trip ) {
  int i;
  trip__clear_cache( trip );
  if ( trip->num_gifts < 1 ) {
    trip->cumulative_distance[0] = 0.0;
    trip->cumulative_weight[0] = 10.0;
    trip->item_distance[0] = 0.0;
    trip->item_weights[0] = 10.0;
    return;
  }
  int num_gifts = trip->num_gifts;

  trip->item_distance[0] = NP_DISTANCE_TO_GIFT( trip->gifts[0] );
  trip->cumulative_distance[0] = trip->item_distance[0];

  for( i = 0; i < num_gifts-1; i++ ) {
    trip->item_distance[i+1] = distance_between_gifts( trip->gifts[i], trip->gifts[i+1] );
    trip->cumulative_distance[i+1] = trip->cumulative_distance[i] + trip->item_distance[i+1];
    trip->item_weights[i] = WEIGHT_OF_GIFT( trip->gifts[i] );
  }

  trip->item_weights[num_gifts-1] = WEIGHT_OF_GIFT( trip->gifts[num_gifts-1] );
  trip->item_distance[num_gifts] = NP_DISTANCE_TO_GIFT( trip->gifts[num_gifts-1] );
  trip->cumulative_distance[num_gifts] = trip->cumulative_distance[num_gifts-1] + trip->item_distance[num_gifts];
  trip->item_weights[num_gifts] = 10.0;

  // Cumulative weight needs to be summed in reverse
  trip->cumulative_weight[num_gifts] = 10.0;
  for( i = num_gifts - 1; i >= 0; i-- ) {
    trip->cumulative_weight[i] = trip->item_weights[i] + trip->cumulative_weight[i+1];
  }

  return;
}

inline void trip__recalc_distances_gdcache( Trip *trip ) {
  int i;
  trip__clear_cache( trip );
  if ( trip->num_gifts < 1 ) {
    trip->cumulative_distance[0] = 0.0;
    trip->cumulative_weight[0] = 10.0;
    trip->item_distance[0] = 0.0;
    trip->item_weights[0] = 10.0;
    return;
  }
  int num_gifts = trip->num_gifts;

  trip->item_distance[0] = NP_DISTANCE_TO_GIFT( trip->gifts[0] );
  trip->cumulative_distance[0] = trip->item_distance[0];

  for( i = 0; i < num_gifts-1; i++ ) {
    trip->item_distance[i+1] = gdcache__distance_between_gifts( trip->gifts[i], trip->gifts[i+1] );
    trip->cumulative_distance[i+1] = trip->cumulative_distance[i] + trip->item_distance[i+1];
    trip->item_weights[i] = WEIGHT_OF_GIFT( trip->gifts[i] );
  }

  trip->item_weights[num_gifts-1] = WEIGHT_OF_GIFT( trip->gifts[num_gifts-1] );
  trip->item_distance[num_gifts] = NP_DISTANCE_TO_GIFT( trip->gifts[num_gifts-1] );
  trip->cumulative_distance[num_gifts] = trip->cumulative_distance[num_gifts-1] + trip->item_distance[num_gifts];
  trip->item_weights[num_gifts] = 10.0;

  // Cumulative weight needs to be summed in reverse
  trip->cumulative_weight[num_gifts] = 10.0;
  for( i = num_gifts - 1; i >= 0; i-- ) {
    trip->cumulative_weight[i] = trip->item_weights[i] + trip->cumulative_weight[i+1];
  }

  return;
}

inline void trip__recalc_cumulative_distances( Trip *trip ) {
  int i;
  trip__clear_cache( trip );
  if ( trip->num_gifts < 1 ) {
    trip->cumulative_distance[0] = 0.0;
    trip->cumulative_weight[0] = 10.0;
    trip->item_distance[0] = 0.0;
    trip->item_weights[0] = 10.0;
    return;
  }
  int num_gifts = trip->num_gifts;

  trip->cumulative_distance[0] = trip->item_distance[0];

  for( i = 0; i < num_gifts-1; i++ ) {
    trip->cumulative_distance[i+1] = trip->cumulative_distance[i] + trip->item_distance[i+1];
  }

  trip->cumulative_distance[num_gifts] = trip->cumulative_distance[num_gifts-1] + trip->item_distance[num_gifts];

  // Cumulative weight needs to be summed in reverse
  trip->cumulative_weight[num_gifts] = 10.0;
  for( i = num_gifts - 1; i >= 0; i-- ) {
    trip->cumulative_weight[i] = trip->item_weights[i] + trip->cumulative_weight[i+1];
  }

  return;
}


Trip *trip__create() {
  Trip *trip;
  trip = xmalloc( sizeof(Trip) );
  trip->num_gifts = 0;
  trip->gifts = NULL;
  trip->cumulative_distance = NULL;
  trip->item_distance = NULL;
  trip->cumulative_weight = NULL;
  trip->item_weights = NULL;
  trip__clear_cache( trip );
  return trip;
}

void trip__init( Trip *trip, int num_gifts ) {
  int i;

  trip->num_gifts = num_gifts;
  trip->gifts = ALLOC_N( int, MAX_TRIP_LENGTH );
  trip->item_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->cumulative_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->item_weights = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->cumulative_weight = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );

  for( i = 0; i < num_gifts; i++ ) {
    trip->gifts[i] = 0;
  }

  trip__recalc_distances( trip );
  return;
}

void trip__init_with_gifts( Trip *trip, int num_gifts, int *new_gifts ) {
  trip->num_gifts = num_gifts;
  trip->gifts = ALLOC_N( int, MAX_TRIP_LENGTH );
  trip->item_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->cumulative_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->item_weights = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip->cumulative_weight = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );

  if ( num_gifts < 1 ) {
    return;
  }

  memcpy( trip->gifts, new_gifts, num_gifts*sizeof(int) );
  trip__recalc_distances( trip );

  return;
}

void trip__set_gifts( Trip *trip, VALUE rv_gifts ) {
  int i, num_gifts;
  struct NARRAY *narr;
  int *narr_gifts_ptr;
  int gift_id;

  GetNArray( rv_gifts, narr );
  narr_gifts_ptr = (int*) narr->ptr;

  num_gifts = narr->shape[0];
  trip__init_with_gifts( trip, num_gifts, narr_gifts_ptr );

  return;
}

void trip__destroy( Trip *trip ) {
  xfree( trip->gifts );
  xfree( trip->cumulative_distance );
  xfree( trip->item_distance );
  xfree( trip->cumulative_weight );
  xfree( trip->item_weights );
  xfree( trip );
  return;
}

void trip__gc_mark( Trip *trip ) {
  return;
}

void trip__deep_copy( Trip *trip_copy, Trip *trip_orig ) {
  trip_copy->num_gifts = trip_orig->num_gifts;
  trip_copy->gifts = ALLOC_N( int, MAX_TRIP_LENGTH );
  trip_copy->item_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip_copy->cumulative_distance = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip_copy->item_weights = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );
  trip_copy->cumulative_weight = ALLOC_N( double, MAX_TRIP_LENGTH_PLUS_ONE );

  memcpy( trip_copy->gifts, trip_orig->gifts, trip_orig->num_gifts * sizeof(int) );
  memcpy( trip_copy->cumulative_distance, trip_orig->cumulative_distance, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->item_weights, trip_orig->item_weights, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->cumulative_weight, trip_orig->cumulative_weight, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->item_distance, trip_orig->item_distance, (trip_orig->num_gifts+1) * sizeof(double) );

  trip_copy->cost = trip_orig->cost;
  trip_copy->efficiency = trip_orig->efficiency;
  return;
}

// Copies assuming that the arrays are already setup
void trip__quick_copy( Trip *trip_copy, Trip *trip_orig ) {
  trip_copy->num_gifts = trip_orig->num_gifts;
  memcpy( trip_copy->gifts, trip_orig->gifts, trip_orig->num_gifts * sizeof(int) );
  memcpy( trip_copy->cumulative_distance, trip_orig->cumulative_distance, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->item_weights, trip_orig->item_weights, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->cumulative_weight, trip_orig->cumulative_weight, (trip_orig->num_gifts+1) * sizeof(double) );
  memcpy( trip_copy->item_distance, trip_orig->item_distance, (trip_orig->num_gifts+1) * sizeof(double) );

  trip_copy->cost = trip_orig->cost;
  trip_copy->efficiency = trip_orig->efficiency;
  return;
}

Trip * trip__clone( Trip *trip_orig ) {
  Trip * trip_copy = trip__create();
  trip__deep_copy( trip_copy, trip_orig );
  return trip_copy;
}

double trip__score( Trip *trip ) {
  if ( trip->cost < 0.0 ) {
    trip->cost = fast_dot_product_sum( trip->num_gifts + 1, trip->item_weights, trip->cumulative_distance );
  }
  return trip->cost;
}

// Weight excludes sleigh weight (of 10.0)
double trip__weight( Trip *trip ) {
  return trip->cumulative_weight[0] - 10.0;
}

double trip__separate_cost( Trip *trip ) {
  int i;
  double cost = 0.0;
  for ( i = 0; i < trip->num_gifts; i++ ) {
    cost += SOLO_SCORE_OF_GIFT( trip->gifts[i] );
  }
  return cost;
}

double trip__efficiency( Trip *trip ) {
  if ( trip->efficiency < 0.0 ) {
    if ( trip->num_gifts > 0 ) {
      trip->efficiency = trip__separate_cost( trip ) / trip__score( trip );
    } else {
      trip->efficiency = 0.0;
    }
  }
  return trip->efficiency;
}

double trip__distance( Trip *trip ) {
  return trip->cumulative_distance[ trip->num_gifts ];
}

static double new_gift_distance_to[151];

// This routine is speed-critical for the optimiser
int trip__lowest_cost_insert_at( Trip *trip, int new_gift_id, double * save_cost ) {
  int i, num_gifts;
  double best_extra_cost = *save_cost;
  int best_insert_id = -1;
  double extra_weight;
  double extra_cost;
  double w;

  num_gifts = trip->num_gifts;
  if ( num_gifts < 1 ) {
    extra_cost = SOLO_SCORE_OF_GIFT( new_gift_id );
    if ( extra_cost < best_extra_cost ) {
      *save_cost = extra_cost;
      best_insert_id =  0;
    }
    return best_insert_id;
  }

  // This is extra cost of insert at start (position 0)
  extra_weight = WEIGHT_OF_GIFT( new_gift_id );
  new_gift_distance_to[num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
  new_gift_distance_to[0] = distance_between_gifts( new_gift_id, trip->gifts[0] );
  w = trip->cumulative_weight[0];
  extra_cost =        ( new_gift_distance_to[num_gifts] * ( extra_weight + w ) )
                    + ( ( new_gift_distance_to[0] - trip->item_distance[0] ) * w );

  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = 0;
  }

  // Extra costs in mid-positions
  for ( i = 1; i < num_gifts; i++ ) {
    extra_cost =      ( trip->cumulative_distance[i-1] * extra_weight );

    // Short-circuit calculations if the excess cost is already exceeding best
    if ( extra_cost > best_extra_cost ) {
      *save_cost = best_extra_cost;
      return best_insert_id;
    }

    new_gift_distance_to[i] = distance_between_gifts( new_gift_id, trip->gifts[i] );
    w = trip->cumulative_weight[i];
    extra_cost +=  ( new_gift_distance_to[i-1] * ( extra_weight + w ) )
                    + ( ( new_gift_distance_to[i] - trip->item_distance[i] ) * w );

    if ( extra_cost < best_extra_cost ) {
      best_extra_cost = extra_cost;
      best_insert_id = i;
    }
  }

  // This is extra cost of insert at end (position num_gifts)
  extra_cost =    ( trip->cumulative_distance[num_gifts-1] * extra_weight )
                + ( new_gift_distance_to[num_gifts-1] * ( extra_weight + 10.0 ) )
                + ( ( new_gift_distance_to[num_gifts] - trip->item_distance[num_gifts] ) * 10.0 );
  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = num_gifts;
  }

  *save_cost = best_extra_cost;

  return best_insert_id;
}

void trip__set_gift( Trip *trip, int idx, int new_gift_id ) {
  trip->gifts[idx] = new_gift_id ;
  // MAYBE: This *could* be optimised by moving stuff around (like add_ below), but it is not
  //        used much except for tests
  trip__recalc_distances( trip );
  return;
}

void trip__add_gift( Trip *trip, int idx, int new_gift_id ) {
  int i;

  if ( trip->num_gifts == 0 ) {
    trip->num_gifts = 1;
    trip->gifts[0] = new_gift_id;
    trip__recalc_distances( trip );
    return;
  }

  memmove( (trip->gifts + idx + 1), (trip->gifts + idx), (trip->num_gifts - idx) * sizeof(int) );
  trip->gifts[idx] = new_gift_id;
  trip->num_gifts++;
  double w = WEIGHT_OF_GIFT( new_gift_id );

  // Adjust magic 10 to decide when recalculating is more efficient than moving the values around . . .
  if ( trip->num_gifts < 10 ) {
    memmove( (trip->item_distance + idx + 1), (trip->item_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
    memmove( (trip->item_weights + idx + 1), (trip->item_weights + idx), (trip->num_gifts - idx) * sizeof(double) );
    trip->item_weights[idx] = w;

    if ( idx ) {
      trip->item_distance[idx] = distance_between_gifts( new_gift_id, trip->gifts[idx-1] );
    } else {
      trip->item_distance[0] = NP_DISTANCE_TO_GIFT( new_gift_id );
    }

    if ( idx + 1 == trip->num_gifts ) {
      trip->item_distance[trip->num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
    } else {
      trip->item_distance[idx+1] = distance_between_gifts( new_gift_id, trip->gifts[idx+1] );
    }

    trip__recalc_cumulative_distances( trip );
    return;
  }

  // This should be more efficient beyond a certain size

  trip__clear_cache( trip );

  // Move up item and cumulative values
  memmove( (trip->item_distance + idx + 1), (trip->item_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->cumulative_distance + idx + 1), (trip->cumulative_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->item_weights + idx + 1), (trip->item_weights + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->cumulative_weight + idx + 1), (trip->cumulative_weight + idx), (trip->num_gifts - idx) * sizeof(double) );

  if ( idx ) {
    trip->item_distance[idx] = distance_between_gifts( new_gift_id, trip->gifts[idx-1] );
    trip->cumulative_distance[idx] = trip->item_distance[idx] + trip->cumulative_distance[idx-1];
  } else {
    trip->item_distance[0] = NP_DISTANCE_TO_GIFT( new_gift_id );
    trip->cumulative_distance[0] = trip->item_distance[0];
  }

  double adjust_distance = -trip->item_distance[idx+1];
  trip->item_weights[idx] = w;
  trip->cumulative_weight[idx] = trip->cumulative_weight[idx+1] + w;

  if ( idx + 1 == trip->num_gifts ) {
    trip->item_distance[trip->num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
  } else {
    trip->item_distance[idx+1] = distance_between_gifts( new_gift_id, trip->gifts[idx+1] );
  }
  adjust_distance += trip->item_distance[idx] + trip->item_distance[idx+1];

  // Adjust cumulative weights and distances
  for ( i = idx - 1; i >= 0; i-- ) {
    trip->cumulative_weight[i] += w;
  }

  for ( i = idx + 1; i <= trip->num_gifts; i++ ) {
    trip->cumulative_distance[i] += adjust_distance;
  }

  return;
}

void trip__remove_gifts_by_idx( Trip *trip, int num, int * idxs ) {
  int i,j,keep;
  int n = 0;
  int * new_gifts = ALLOC_N( int, MAX_TRIP_LENGTH );

  for ( i = 0; i < trip->num_gifts; i++ ) {
    keep = 1;
    for ( j = 0; j < num; j ++ ) {
      if ( i == idxs[j] ) {
        keep = 0;
        break;
      }
    }
    if (keep) {
      new_gifts[n] = trip->gifts[i];
      n++;
    }
  }

  trip->num_gifts = n;

  xfree( trip->gifts );
  trip->gifts = new_gifts;
  trip__recalc_distances( trip );
  return;
}

void trip__remove_gifts_by_gift_id( Trip *trip, int num, int * gift_ids ) {
  int i,j,keep,gift_id;
  int n = 0;
  int * new_gifts = ALLOC_N( int, MAX_TRIP_LENGTH );

  for ( i = 0; i < trip->num_gifts; i++ ) {
    keep = 1;
    gift_id = trip->gifts[i];
    for ( j = 0; j < num; j ++ ) {
      if ( gift_id == gift_ids[j] ) {
        keep = 0;
        break;
      }
    }
    if (keep) {
      new_gifts[n] = gift_id;
      n++;
    }
  }

  trip->num_gifts = n;

  xfree( trip->gifts );
  trip->gifts = new_gifts;
  trip__recalc_distances( trip );
  return;
}

// Private function . . .
static int scratch_sample[ 1000 ];
int trip__load_remaining_gifts_after_sample( Trip *trip_src, Trip *trip_remaining, int num, int * gift_ids ) {
  int i,j,keep,gift_id;
  int n = 0;
  int nsampled = 0;
  int * new_gifts = trip_remaining->gifts;
  for ( i = 0; i < trip_src->num_gifts; i++ ) {
    keep = 1;
    gift_id = trip_src->gifts[i];
    for ( j = 0; j < num; j ++ ) {
      if ( gift_id == scratch_sample[j] ) {
        keep = 0;
        gift_ids[nsampled] = gift_id;
        nsampled++;
        break;
      }
    }
    if (keep) {
      new_gifts[n] = gift_id;
      n++;
    }
  }
  trip_remaining->num_gifts = n;
  trip__recalc_distances( trip_remaining );
  return nsampled;
}

int trip__sample_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample ) {
  int i;
  if ( trip_src->num_gifts == 0 ) {
    trip_remaining->num_gifts = 0;
    trip__recalc_distances( trip_remaining );
    return 0;
  }
  for ( i = 0; i < max_sample; i++ ) {
    scratch_sample[i] = trip_src->gifts[ ( genrand_int31() % trip_src->num_gifts ) ];
  }
  return trip__load_remaining_gifts_after_sample( trip_src, trip_remaining, max_sample, sample );
}

int trip__sample_gifts_var( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample ) {
  return trip__sample_gifts( trip_src, ( 1 + ( genrand_int31() % max_sample ) ), trip_remaining, sample );
}

double trip__total_tension( Trip *trip ) {
  int i, gift_id;
  double t = 0.0;
  for ( i = 0; i < trip->num_gifts; i++ ) {
    gift_id = trip->gifts[i];
    t +=  WEIGHT_OF_GIFT( gift_id ) * ( trip->cumulative_distance[i] - NP_DISTANCE_TO_GIFT(gift_id) );
  }
  return t;
}

// tensions should be a 100,000-element array
// Tension is forward-cost of gift in a trip minus the forward-cost of gift
void trip__calc_gift_tensions( Trip *trip, double * tensions ) {
  int i, gift_id;
  for ( i = 0; i < trip->num_gifts; i++ ) {
    gift_id = trip->gifts[i];
    tensions[gift_id] =  WEIGHT_OF_GIFT( gift_id ) * ( trip->cumulative_distance[i] - NP_DISTANCE_TO_GIFT(gift_id) );
  }
}

int trip__sample_start_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample ) {
  int i;
  if ( trip_src->num_gifts == 0 ) {
    trip_remaining->num_gifts = 0;
    trip__recalc_distances( trip_remaining );
    return 0;
  }
  if ( trip_src->num_gifts < max_sample ) {
    max_sample = trip_src->num_gifts;
  }
  for ( i = 0; i < max_sample; i++ ) {
    scratch_sample[i] = trip_src->gifts[ i ];
  }
  return trip__load_remaining_gifts_after_sample( trip_src, trip_remaining, max_sample, sample );
}

int trip__sample_end_gifts( Trip *trip_src, int max_sample, Trip *trip_remaining, int * sample ) {
  int i;
  if ( trip_src->num_gifts == 0 ) {
    trip_remaining->num_gifts = 0;
    trip__recalc_distances( trip_remaining );
    return 0;
  }
  if ( trip_src->num_gifts < max_sample ) {
    max_sample = trip_src->num_gifts;
  }
  for ( i = 0; i < max_sample; i++ ) {
    scratch_sample[i] = trip_src->gifts[ trip_src->num_gifts - i - 1 ];
  }
  return trip__load_remaining_gifts_after_sample( trip_src, trip_remaining, max_sample, sample );
}

int trip__lowest_cost_break_at( Trip *trip, double * save_cost ) {
  // A break point of 0 means keep as-is (i.e. break *before* gift 0, and new trip)
  int i, j, best_p = 0;
  double this_cost, cost_pre, cost_post, pre_cumulative_weight, post_cumulative_distance, best_cost = 0.0;

  for ( i = 1; i < trip->num_gifts; i++ ) {
    // Gifts 0 to i-1 are in one trip, gifts i to num_gifts-1 are in other trip
    // Before split point gets new cumulative weights and ends with gift i-1
    // After split point gets new cumulative distances and starts with gift i

    // Calc trip cost pre-split
    pre_cumulative_weight = 10.0; // For the sleigh
    cost_pre = pre_cumulative_weight * NP_DISTANCE_TO_GIFT( trip->gifts[i-1] );
    for ( j = i-1; j >= 0; j-- ) {
      pre_cumulative_weight += trip->item_weights[j];
      cost_pre += pre_cumulative_weight * trip->item_distance[j];
    }

    // Calc trip cost post-split
    post_cumulative_distance = NP_DISTANCE_TO_GIFT( trip->gifts[i] );
    cost_post = post_cumulative_distance * trip->item_weights[i];
    for ( j = i+1; j <= trip->num_gifts; j++ ) {
      post_cumulative_distance += trip->item_distance[j];
      cost_post += post_cumulative_distance * trip->item_weights[j];
    }

    this_cost = cost_pre + cost_post - trip__score( trip );

    if ( this_cost < best_cost ) {
      best_p = i;
      best_cost = this_cost;
    }
  }

  *save_cost = best_cost;
  return best_p;
}


void trip__splice( Trip *trip, int start_idx, int num_splice_out, Trip *input_trip, Trip *output_trip ) {
  int i, end_splice = 0;

  if ( start_idx >= trip->num_gifts ) {
    start_idx = trip->num_gifts;
    end_splice = 1;
    num_splice_out = 0;
  }

  if ( start_idx + num_splice_out > trip->num_gifts ) {
    num_splice_out = trip->num_gifts - start_idx;
  }

  // Copy spliced values to output trip
  if ( num_splice_out > 0 ) {

    output_trip->num_gifts = num_splice_out;
    memcpy( output_trip->gifts, trip->gifts + start_idx, num_splice_out * sizeof(int) );
    memcpy( output_trip->item_distance, trip->item_distance + start_idx, num_splice_out * sizeof(double) );
    memcpy( output_trip->item_weights, trip->item_weights + start_idx, num_splice_out * sizeof(double) );

    output_trip->item_distance[0] = NP_DISTANCE_TO_GIFT( trip->gifts[ start_idx ] );
    output_trip->item_distance[num_splice_out] = NP_DISTANCE_TO_GIFT( trip->gifts[ start_idx + num_splice_out - 1 ] );
    output_trip->item_weights[num_splice_out] = 10.0;
    trip__recalc_cumulative_distances( output_trip );
  } else {
    output_trip->num_gifts = 0;
    trip__recalc_distances( output_trip );
  }

  // Move up existing item values
  i = trip->num_gifts - start_idx - num_splice_out;
  memmove( (trip->gifts + start_idx + input_trip->num_gifts), (trip->gifts + start_idx + num_splice_out),
             i * sizeof(int) );
  memmove( (trip->item_distance + start_idx + input_trip->num_gifts), (trip->item_distance + start_idx + num_splice_out),
             (i + 1) * sizeof(double) );
  memmove( (trip->item_weights + start_idx + input_trip->num_gifts), (trip->item_weights + start_idx + num_splice_out),
             (i + 1) * sizeof(double) );

  // Copy in new values
  if ( input_trip->num_gifts > 0 ) {
    memcpy( trip->gifts + start_idx, input_trip->gifts, input_trip->num_gifts * sizeof(int) );
    memcpy( trip->item_distance + start_idx, input_trip->item_distance, (input_trip->num_gifts ) * sizeof(double) );
    memcpy( trip->item_weights + start_idx, input_trip->item_weights, ( input_trip->num_gifts ) * sizeof(double) );
  }

  trip->num_gifts = trip->num_gifts - num_splice_out + input_trip->num_gifts;

  // Fix distance "joins" at [start_idx] and [start_idx + input_trip->num_gifts + 1]
  if ( start_idx > 0 ) {
    if ( trip->num_gifts > start_idx ) {
      trip->item_distance[start_idx] = distance_between_gifts( trip->gifts[start_idx-1], trip->gifts[start_idx] );
    }
  } else {
    if ( trip->num_gifts > 0 ) {
      trip->item_distance[0] = NP_DISTANCE_TO_GIFT( trip->gifts[ 0 ] );
    } else {
      trip->item_distance[0] = 0.0;
    }
  }

  i = start_idx + input_trip->num_gifts;
  if ( i < trip->num_gifts ) {
    if ( i > 0 ) {
      trip->item_distance[i] = distance_between_gifts( trip->gifts[i-1], trip->gifts[i] );
    }
  } else if ( trip->num_gifts > 0 ) {
    trip->item_distance[trip->num_gifts] = NP_DISTANCE_TO_GIFT( trip->gifts[ trip->num_gifts - 1 ] );
  }

  trip__recalc_cumulative_distances( trip );

  return;
}


void trip__mutual_splice( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a, Trip *trip_b, int idx_out_b, int num_b, int idx_in_b ) {
  // TODO: These could be a static buffer
  Trip * tmp_trip_out_a = trip__create();
  Trip * tmp_trip_in_a = trip__create();
  trip__init( tmp_trip_out_a, 0 );
  trip__init( tmp_trip_in_a, 0 );

  Trip * tmp_trip_out_b = trip__create();
  Trip * tmp_trip_in_b = trip__create();
  trip__init( tmp_trip_out_b, 0 );
  trip__init( tmp_trip_in_b, 0 );

  // TODO: This could be made more efficient (less calls, less temp vars)
  trip__splice( trip_a, idx_out_a, num_a, tmp_trip_in_a, tmp_trip_out_a );
  trip__splice( trip_b, idx_out_b, num_b, tmp_trip_in_b, tmp_trip_out_b );

  if ( ( trip__weight( trip_a ) + trip__weight( tmp_trip_out_b ) ) <= 1000.0  &&
       ( trip__weight( trip_b ) + trip__weight( tmp_trip_out_a ) ) <= 1000.0
      ) {
    trip__splice( trip_a, idx_in_a, 0, tmp_trip_out_b, tmp_trip_in_b );
    trip__splice( trip_b, idx_in_b, 0, tmp_trip_out_a, tmp_trip_in_a );
  } else {
    trip__splice( trip_a, idx_out_a, 0, tmp_trip_out_a, tmp_trip_in_a );
    trip__splice( trip_b, idx_out_b, 0, tmp_trip_out_b, tmp_trip_in_b );
  }

  trip__destroy( tmp_trip_in_a );
  trip__destroy( tmp_trip_out_a );

  trip__destroy( tmp_trip_in_b );
  trip__destroy( tmp_trip_out_b );

  return;
}

void trip__splice_from_a_to_b( Trip *trip_a, int idx_out_a, int num_a, Trip *trip_b, int idx_in_b ) {
  // TODO: These could be a static buffer
  Trip * tmp_trip_out_a = trip__create();
  trip__init( tmp_trip_out_a, 0 );

  Trip * tmp_trip_in_b = trip__create();
  trip__init( tmp_trip_in_b, 0 );

  // TODO: This could be made more efficient (less calls, less temp vars)
  trip__splice( trip_a, idx_out_a, num_a, tmp_trip_in_b, tmp_trip_out_a );

  if ( ( trip__weight( trip_b ) + trip__weight( tmp_trip_out_a ) ) <= 1000.0 ) {
    trip__splice( trip_b, idx_in_b, 0, tmp_trip_out_a, tmp_trip_in_b );
  } else {
    // NB This is a silent fail
    trip__splice( trip_a, idx_out_a, 0, tmp_trip_out_a, tmp_trip_in_b );
  }

  trip__destroy( tmp_trip_out_a );
  trip__destroy( tmp_trip_in_b );

  return;
}

void trip__splice_self( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a ) {
  // TODO: These could be a static buffer
  Trip * tmp_trip_out_a = trip__create();
  trip__init( tmp_trip_out_a, 0 );

  Trip * tmp_trip_in_a = trip__create();
  trip__init( tmp_trip_in_a, 0 );

  // TODO: This could be made more efficient (less calls, less temp vars)
  trip__splice( trip_a, idx_out_a, num_a, tmp_trip_in_a, tmp_trip_out_a );
  trip__splice( trip_a, idx_in_a, 0, tmp_trip_out_a, tmp_trip_in_a );

  trip__destroy( tmp_trip_out_a );
  trip__destroy( tmp_trip_in_a );

  return;
}

static int gifts_a[MAX_TRIP_LENGTH];
static int gifts_b[MAX_TRIP_LENGTH];
static double dist_a[MAX_TRIP_LENGTH_PLUS_ONE];
static double cdist_a[MAX_TRIP_LENGTH_PLUS_ONE];
static double weight_a[MAX_TRIP_LENGTH_PLUS_ONE];
static double dist_b[MAX_TRIP_LENGTH_PLUS_ONE];
static double cdist_b[MAX_TRIP_LENGTH_PLUS_ONE];
static double weight_b[MAX_TRIP_LENGTH_PLUS_ONE];

inline void mutual_splice_process_doubles( double * dest, double * src_a, double * src_b, int size_a, int idx_out_a, int num_a, int idx_in_a, int idx_out_b, int num_b ) {
  memcpy( dest, src_a, idx_out_a * sizeof(double) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a + 1) * sizeof(double) );
  // Insert trip_b's weights into a's temp data
  memmove( dest + idx_in_a + num_b, dest + idx_in_a, ( size_a - num_a - idx_in_a + 1 ) * sizeof(double) );
  memcpy( dest + idx_in_a, src_b + idx_out_b, num_b * sizeof(double) );
  return;
}

inline void mutual_splice_process_ints( int * dest, int * src_a, int * src_b, int size_a, int idx_out_a, int num_a, int idx_in_a, int idx_out_b, int num_b ) {
  memcpy( dest, src_a, idx_out_a * sizeof(int) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a ) * sizeof(int) );
  // Insert trip_b's weights into a's temp data
  memmove( dest + idx_in_a + num_b, dest + idx_in_a, ( size_a  - num_a - idx_in_a  ) * sizeof(int) );
  memcpy( dest + idx_in_a, src_b + idx_out_b, num_b * sizeof(int) );
  return;
}

// FIXME: this cannot cope with either trip ending up at 0 length
double trip__mutual_splice_score( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a, Trip *trip_b, int idx_out_b, int num_b, int idx_in_b ) {
  int i;
  int new_len_a = trip_a->num_gifts - num_a + num_b;
  int new_len_b = trip_b->num_gifts - num_b + num_a;
  double score, tw;

  // Copy trip_a weights to a's temp data
  mutual_splice_process_doubles( weight_a, trip_a->item_weights, trip_b->item_weights, trip_a->num_gifts,
                                 idx_out_a, num_a, idx_in_a, idx_out_b, num_b );
  tw = 0.0;
  for ( i = 0; i < new_len_a; i++ ) {
    tw += weight_a[i];
  }
  if ( tw > 1000.0 ) {
    return 100000000000000.0;
  }

  // Copy trip_b weights to b's temp data
  mutual_splice_process_doubles( weight_b, trip_b->item_weights, trip_a->item_weights, trip_b->num_gifts,
                                 idx_out_b, num_b, idx_in_b, idx_out_a, num_a );

  tw = 0.0;
  for ( i = 0; i < new_len_b; i++ ) {
    tw += weight_b[i];
  }
  if ( tw > 1000.0 ) {
    return 100000000000000.0;
  }

  // Copy gifts
  mutual_splice_process_ints( gifts_a, trip_a->gifts, trip_b->gifts, trip_a->num_gifts,
                               idx_out_a, num_a, idx_in_a, idx_out_b, num_b );
  mutual_splice_process_ints( gifts_b, trip_b->gifts, trip_a->gifts, trip_b->num_gifts,
                               idx_out_b, num_b, idx_in_b, idx_out_a, num_a );

  // Calculate distances
  dist_a[0] = NP_DISTANCE_TO_GIFT( gifts_a[0] );
  for( i = 1; i < new_len_a; i++ ) {
    // Note this relies on gdcache
    dist_a[i] = gdcache__distance_between_gifts( gifts_a[i-1], gifts_a[i] );
  }
  dist_a[new_len_a] = NP_DISTANCE_TO_GIFT( gifts_a[new_len_a-1] );

  dist_b[0] = NP_DISTANCE_TO_GIFT( gifts_b[0] );
  for( i = 1; i < new_len_b; i++ ) {
    // Note this relies on gdcache
    dist_b[i] = gdcache__distance_between_gifts( gifts_b[i-1], gifts_b[i] );
  }
  dist_b[new_len_b] = NP_DISTANCE_TO_GIFT( gifts_b[new_len_b-1] );

  // Cumulative distances
  tw = 0.0;
  for ( i = 0; i <= new_len_a; i++ ) {
    tw += dist_a[i];
    cdist_a[i] = tw;
    // printf( "A[%d]: gift_id %d, weight x cdist ( %f * %f )\n", i, gifts_a[i], weight_a[i], cdist_a[i] );
  }
  tw = 0.0;
  for ( i = 0; i <= new_len_b; i++ ) {
    tw += dist_b[i];
    cdist_b[i] = tw;
    // printf( "B[%d]: gift_id %d, weight x cdist ( %f * %f )\n", i, gifts_b[i], weight_b[i], cdist_b[i] );
  }

  score =   fast_dot_product_sum( new_len_a+1, cdist_a, weight_a )
          + fast_dot_product_sum( new_len_b+1, cdist_b, weight_b );

  return score;
}


inline void a_to_b_splice_process_a_doubles( double * dest, double * src_a, int size_a, int idx_out_a, int num_a ) {
  memcpy( dest, src_a, idx_out_a * sizeof(double) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a + 1) * sizeof(double) );
  return;
}

inline void a_to_b_splice_process_a_ints( int * dest, int * src_a, int size_a, int idx_out_a, int num_a ) {
  memcpy( dest, src_a, idx_out_a * sizeof(int) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a ) * sizeof(int) );
  return;
}

inline void a_to_b_splice_process_b_doubles( double * dest, double * src_a, double * src_b, int size_b, int idx_out_a, int num_a, int idx_in_b ) {
  memcpy( dest, src_b, idx_in_b * sizeof(double) );
  memcpy( dest + idx_in_b + num_a, src_b + idx_in_b, ( size_b - idx_in_b + 1) * sizeof(double) );
  // Insert trip_a's weights into b's temp data
  memcpy( dest + idx_in_b, src_a + idx_out_a, num_a * sizeof(double) );
  return;
}

inline void a_to_b_splice_process_b_ints( int * dest, int * src_a, int * src_b, int size_b, int idx_out_a, int num_a, int idx_in_b ) {
  memcpy( dest, src_b, idx_in_b * sizeof(int) );
  memcpy( dest + idx_in_b + num_a, src_b + idx_in_b, ( size_b - idx_in_b ) * sizeof(int) );
  // Insert trip_a's weights into b's temp data
  memcpy( dest + idx_in_b, src_a + idx_out_a, num_a * sizeof(int) );
  return;
}

double trip__splice_from_a_to_b_score( Trip *trip_a, int idx_out_a, int num_a, Trip *trip_b, int idx_in_b ) {
  int i;
  int new_len_a = trip_a->num_gifts - num_a;
  int new_len_b = trip_b->num_gifts + num_a;
  double score, tw;

  // Copy trip_a weights to b's temp data
  a_to_b_splice_process_b_doubles( weight_b, trip_a->item_weights, trip_b->item_weights, trip_b->num_gifts,
                                   idx_out_a, num_a, idx_in_b );
  tw = 0.0;
  for ( i = 0; i < new_len_b; i++ ) {
    tw += weight_b[i];
  }
  if ( tw > 1000.0 ) {
    return 100000000000000.0;
  }

  // Rmove items from a
  a_to_b_splice_process_a_doubles( weight_a, trip_a->item_weights, trip_a->num_gifts, idx_out_a, num_a );

  // Copy gifts
  a_to_b_splice_process_a_ints( gifts_a, trip_a->gifts, trip_a->num_gifts, idx_out_a, num_a );
  a_to_b_splice_process_b_ints( gifts_b, trip_a->gifts, trip_b->gifts, trip_b->num_gifts,
                                 idx_out_a, num_a, idx_in_b );

  // Calculate distances
  if ( new_len_a > 0 ) {
    dist_a[0] = NP_DISTANCE_TO_GIFT( gifts_a[0] );
    for( i = 1; i < new_len_a; i++ ) {
      // Note this relies on gdcache
      dist_a[i] = gdcache__distance_between_gifts( gifts_a[i-1], gifts_a[i] );
    }
    dist_a[new_len_a] = NP_DISTANCE_TO_GIFT( gifts_a[new_len_a-1] );
  } else {
    dist_a[0] = 0.0;
  }

  dist_b[0] = NP_DISTANCE_TO_GIFT( gifts_b[0] );
  for( i = 1; i < new_len_b; i++ ) {
    // Note this relies on gdcache
    dist_b[i] = gdcache__distance_between_gifts( gifts_b[i-1], gifts_b[i] );
  }
  dist_b[new_len_b] = NP_DISTANCE_TO_GIFT( gifts_b[new_len_b-1] );

  // Cumulative distances
  tw = 0.0;
  for ( i = 0; i <= new_len_a; i++ ) {
    tw += dist_a[i];
    cdist_a[i] = tw;
    // printf( "A[%d]: gift_id %d, weight x cdist ( %f * %f )\n", i, gifts_a[i], weight_a[i], cdist_a[i] );
  }
  tw = 0.0;
  for ( i = 0; i <= new_len_b; i++ ) {
    tw += dist_b[i];
    cdist_b[i] = tw;
    // printf( "B[%d]: gift_id %d, weight x cdist ( %f * %f )\n", i, gifts_b[i], weight_b[i], cdist_b[i] );
  }

  score =   fast_dot_product_sum( new_len_a+1, cdist_a, weight_a )
          + fast_dot_product_sum( new_len_b+1, cdist_b, weight_b );

  return score;
}

inline void self_splice_process_doubles( double * dest, double * src_a, int size_a, int idx_out_a, int num_a, int idx_in_a ) {
  memcpy( dest, src_a, idx_out_a * sizeof(double) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a + 1) * sizeof(double) );
  // Copy back in
  memmove( dest + idx_in_a + num_a, dest + idx_in_a, ( size_a - num_a - idx_in_a + 1 ) * sizeof(double) );
  memcpy( dest + idx_in_a, src_a + idx_out_a, num_a * sizeof(double) );
  return;
}

inline void self_splice_process_ints( int * dest, int * src_a, int size_a, int idx_out_a, int num_a, int idx_in_a ) {
  memcpy( dest, src_a, idx_out_a * sizeof(int) );
  memcpy( dest + idx_out_a, src_a + idx_out_a + num_a, ( size_a - idx_out_a - num_a ) * sizeof(int) );
  // Copy back in
  memmove( dest + idx_in_a + num_a, dest + idx_in_a, ( size_a - num_a - idx_in_a + 1 ) * sizeof(int) );
  memcpy( dest + idx_in_a, src_a + idx_out_a, num_a * sizeof(int) );
  return;
}

double trip__splice_self_score( Trip *trip_a, int idx_out_a, int num_a, int idx_in_a ) {
  int i;
  int new_len_a = trip_a->num_gifts;
  double score, tw;

  // Rmove items from a
  self_splice_process_doubles( weight_a, trip_a->item_weights, trip_a->num_gifts, idx_out_a, num_a, idx_in_a );

  // Copy gifts
  self_splice_process_ints( gifts_a, trip_a->gifts, trip_a->num_gifts, idx_out_a, num_a, idx_in_a );

  // Calculate distances
  if ( new_len_a > 0 ) {
    dist_a[0] = NP_DISTANCE_TO_GIFT( gifts_a[0] );
    for( i = 1; i < new_len_a; i++ ) {
      // Note this relies on gdcache
      dist_a[i] = gdcache__distance_between_gifts( gifts_a[i-1], gifts_a[i] );
    }
    dist_a[new_len_a] = NP_DISTANCE_TO_GIFT( gifts_a[new_len_a-1] );
  } else {
    dist_a[0] = 0.0;
  }

  // Cumulative distances
  tw = 0.0;
  for ( i = 0; i <= new_len_a; i++ ) {
    tw += dist_a[i];
    cdist_a[i] = tw;
    // printf( "A[%d]: gift_id %d, weight x cdist ( %f * %f )\n", i, gifts_a[i], weight_a[i], cdist_a[i] );
  }

  score = fast_dot_product_sum( new_len_a+1, cdist_a, weight_a );

  return score;
}



int trip__lowest_cost_insert_at_gdcache( Trip *trip, int new_gift_id, double * save_cost ) {
  int i, num_gifts;
  double best_extra_cost = *save_cost;
  int best_insert_id = -1;
  double extra_weight;
  double extra_cost;
  double w;

  num_gifts = trip->num_gifts;
  if ( num_gifts < 1 ) {
    extra_cost = SOLO_SCORE_OF_GIFT( new_gift_id );
    if ( extra_cost < best_extra_cost ) {
      *save_cost = extra_cost;
      best_insert_id =  0;
    }
    return best_insert_id;
  }

  // This is extra cost of insert at start (position 0)
  extra_weight = WEIGHT_OF_GIFT( new_gift_id );
  new_gift_distance_to[num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
  new_gift_distance_to[0] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[0] );
  w = trip->cumulative_weight[0];
  extra_cost =        ( new_gift_distance_to[num_gifts] * ( extra_weight + w ) )
                    + ( ( new_gift_distance_to[0] - trip->item_distance[0] ) * w );

  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = 0;
  }

  // Extra costs in mid-positions
  for ( i = 1; i < num_gifts; i++ ) {
    extra_cost =      ( trip->cumulative_distance[i-1] * extra_weight );

    // Short-circuit calculations if the excess cost is already exceeding best
    if ( extra_cost > best_extra_cost ) {
      *save_cost = best_extra_cost;
      return best_insert_id;
    }

    new_gift_distance_to[i] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[i] );
    w = trip->cumulative_weight[i];
    extra_cost +=  ( new_gift_distance_to[i-1] * ( extra_weight + w ) )
                    + ( ( new_gift_distance_to[i] - trip->item_distance[i] ) * w );

    if ( extra_cost < best_extra_cost ) {
      best_extra_cost = extra_cost;
      best_insert_id = i;
    }
  }

  // This is extra cost of insert at end (position num_gifts)
  extra_cost =    ( trip->cumulative_distance[num_gifts-1] * extra_weight )
                + ( new_gift_distance_to[num_gifts-1] * ( extra_weight + 10.0 ) )
                + ( ( new_gift_distance_to[num_gifts] - trip->item_distance[num_gifts] ) * 10.0 );
  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = num_gifts;
  }

  *save_cost = best_extra_cost;

  return best_insert_id;
}

void trip__add_gift_gdcache( Trip *trip, int idx, int new_gift_id ) {
  int i;

  if ( trip->num_gifts == 0 ) {
    trip->num_gifts = 1;
    trip->gifts[0] = new_gift_id;
    trip__recalc_distances( trip );
    return;
  }

  memmove( (trip->gifts + idx + 1), (trip->gifts + idx), (trip->num_gifts - idx) * sizeof(int) );
  trip->gifts[idx] = new_gift_id;
  trip->num_gifts++;
  double w = WEIGHT_OF_GIFT( new_gift_id );

  // Adjust magic 10 to decide when recalculating is more efficient than moving the values around . . .
    memmove( (trip->item_distance + idx + 1), (trip->item_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
    memmove( (trip->item_weights + idx + 1), (trip->item_weights + idx), (trip->num_gifts - idx) * sizeof(double) );
    trip->item_weights[idx] = w;

    if ( idx ) {
      trip->item_distance[idx] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[idx-1] );
    } else {
      trip->item_distance[0] = NP_DISTANCE_TO_GIFT( new_gift_id );
    }

    if ( idx + 1 == trip->num_gifts ) {
      trip->item_distance[trip->num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
    } else {
      trip->item_distance[idx+1] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[idx+1] );
    }

    trip__recalc_cumulative_distances( trip );
    return;

  // This should be more efficient beyond a certain size (it isn't - currently unreachable)
  /*
  trip__clear_cache( trip );

  // Move up item and cumulative values
  memmove( (trip->item_distance + idx + 1), (trip->item_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->cumulative_distance + idx + 1), (trip->cumulative_distance + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->item_weights + idx + 1), (trip->item_weights + idx), (trip->num_gifts - idx) * sizeof(double) );
  memmove( (trip->cumulative_weight + idx + 1), (trip->cumulative_weight + idx), (trip->num_gifts - idx) * sizeof(double) );

  if ( idx ) {
    trip->item_distance[idx] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[idx-1] );
    trip->cumulative_distance[idx] = trip->item_distance[idx] + trip->cumulative_distance[idx-1];
  } else {
    trip->item_distance[0] = NP_DISTANCE_TO_GIFT( new_gift_id );
    trip->cumulative_distance[0] = trip->item_distance[0];
  }

  double adjust_distance = -trip->item_distance[idx+1];
  trip->item_weights[idx] = w;
  trip->cumulative_weight[idx] = trip->cumulative_weight[idx+1] + w;

  if ( idx + 1 == trip->num_gifts ) {
    trip->item_distance[trip->num_gifts] = NP_DISTANCE_TO_GIFT( new_gift_id );
  } else {
    trip->item_distance[idx+1] = gdcache__distance_between_gifts( new_gift_id, trip->gifts[idx+1] );
  }
  adjust_distance += trip->item_distance[idx] + trip->item_distance[idx+1];

  // Adjust cumulative weights and distances
  for ( i = idx - 1; i >= 0; i-- ) {
    trip->cumulative_weight[i] += w;
  }

  for ( i = idx + 1; i <= trip->num_gifts; i++ ) {
    trip->cumulative_distance[i] += adjust_distance;
  }

  return;
  */
}
