// ext/stolen_sleigh/utilities.c

#include "utilities.h"

int id_out_of_bounds( int id ) {
  if ( id < 0 || id > 99999 ) {
    return 1;
  }
  return 0;
}

double haversine_distance( double lat_a, double lng_a, double lat_b, double lng_b ) {
  double dx, dy;

  dx = sin( (lat_a - lat_b) / 2);
  dx *= dx;

  dy = sin( (lng_a - lng_b) / 2);
  dy *= dy;
  dy *= cos(lat_a) * cos(lat_b);

  return asin(sqrt(dx + dy)) * EARTH_DIAMETER;
}

inline double fast_haversine_distance( double a_x, double a_y, double a_z, double b_x, double b_y, double b_z ) {
    double dx = a_x - b_x;
    double dy = a_y - b_y;
    double dz = a_z - b_z;
    return EARTH_DIAMETER * asin( sqrt(dx*dx + dy*dy + dz*dz) );
}

double distance_between_gifts( int gift_a_id, int gift_b_id ) {
  return fast_haversine_distance( X_OF_GIFT( gift_a_id ), Y_OF_GIFT( gift_a_id ), Z_OF_GIFT( gift_a_id ),
                                  X_OF_GIFT( gift_b_id ), Y_OF_GIFT( gift_b_id ), Z_OF_GIFT( gift_b_id ) );
}

double distance_between_gift_and_longitude( int gift_id, double lng ) {
  return haversine_distance( LAT_OF_GIFT( gift_id ), LONG_OF_GIFT( gift_id ), LAT_OF_GIFT( gift_id ), lng );
}

double cylinder_distance_between_gift_and_longitude( int gift_id, double lng ) {
  double dy;

  dy = sin( (LONG_OF_GIFT( gift_id ) - lng) / 2);
  dy *= dy;

  return asin( sqrt(dy) ) * EARTH_DIAMETER;
}

// Note this is not symmetric, gift_b sets longitude for measuring closeness, so a list of
// gifts "closest" to b can be made
double weighted_metric_distance_between_gifts( int gift_a_id, int gift_b_id, double w_hav, double w_long_only, double w_cyl ) {
  return   w_hav * distance_between_gifts( gift_a_id, gift_b_id )
         + w_long_only * distance_between_gift_and_longitude( gift_a_id, LONG_OF_GIFT( gift_b_id ) )
         + w_cyl * cylinder_distance_between_gift_and_longitude( gift_a_id, LONG_OF_GIFT( gift_b_id ) );
}

// Returns distance squared which can be used to find closest
double cartesian_distance_metric( int gift_a_id, int gift_b_id ) {
  double dx = X_OF_GIFT( gift_a_id ) -  X_OF_GIFT( gift_b_id );
  double dy = Y_OF_GIFT( gift_a_id ) -  Y_OF_GIFT( gift_b_id );
  double dz = Z_OF_GIFT( gift_a_id ) -  Z_OF_GIFT( gift_b_id );
  return dx * dx + dy * dy + dz * dz;
}

char * create_id_tracker() {
  int i;
  char * id_tracker = ALLOC_N( char, 100000 );
  for ( i = 0; i < 100000; i++ ) {
    id_tracker[i] = 0;
  }
  return id_tracker;
}

void destroy_id_tracker( char * id_tracker ) {
  xfree( id_tracker );
  return;
}

void quicksort_ids_by_double( int * ids, double * sort_by, int lowest, int highest ) {
  int pivot, j, i;
  int temp_id;

  if ( lowest < highest ) {
    pivot = lowest;
    i = lowest;
    j = highest;

    // Calculate pivot
    while ( i < j ) {
      while ( sort_by[ids[i]] <= sort_by[ids[pivot]] && i < highest )
        i++;
      while ( sort_by[ids[j]] > sort_by[ids[pivot]] )
        j--;
      if( i < j ) {
        temp_id = ids[i];
        ids[i] = ids[j];
        ids[j] = temp_id;
      }
    }

    temp_id = ids[pivot];
    ids[pivot] = ids[j];
    ids[j] = temp_id;

    quicksort_ids_by_double( ids, sort_by, lowest, j-1 );
    quicksort_ids_by_double( ids, sort_by, j+1, highest );
  }
}

double trip_outward_distance( int num_gifts, int * gift_ids ) {
  double d = 0.0;
  int i;

  if (num_gifts < 1) {
    return d;
  }
  // Distance to first gift
  d = NP_DISTANCE_TO_GIFT( gift_ids[0] );
  if (num_gifts == 1) {
    return d;
  }

  for ( i = 1; i < num_gifts; i++ ) {
    d += distance_between_gifts( gift_ids[i-1], gift_ids[i] );
  }

  return d;
}

double trip_cost( int num_gifts,  int * gift_ids ) {
  int i;
  if (num_gifts < 1) {
    return 0.0;
  }

  double current_weight = 10.0;
  double cost = 0.0;

  // This works through trip in reverse. First and last items are special cases where journey is to North Pole
  cost = current_weight * NP_DISTANCE_TO_GIFT( gift_ids[num_gifts-1] );
  current_weight += WEIGHT_OF_GIFT( gift_ids[num_gifts-1] );

  for ( i = num_gifts - 2; i >= 0; i-- ) {
    cost += current_weight * distance_between_gifts( gift_ids[i], gift_ids[i+1] );
    current_weight += WEIGHT_OF_GIFT( gift_ids[i] );
  }

  return cost + current_weight * NP_DISTANCE_TO_GIFT( gift_ids[0] );
}

double trip_total_weight( int num_gifts, int * gift_ids ) {
  int i;
  double weight = 0.0;
  for ( i = num_gifts-1; i >= 0; i-- ) {
    weight += WEIGHT_OF_GIFT( gift_ids[i] );
  }
  return weight;
}

double trip_separate_cost( int num_gifts, int * gift_ids ) {
  int i;
  double cost = 0.0;
  for ( i = num_gifts-1; i >= 0; i-- ) {
    cost += SOLO_SCORE_OF_GIFT( gift_ids[i] );
  }
  return cost;
}

double trip_efficiency( int num_gifts, int * gift_ids ) {
  if ( num_gifts < 2 ) {
    return 1.0;
  }
  return trip_separate_cost( num_gifts, gift_ids ) / trip_cost( num_gifts, gift_ids );
}

double trip_distance( int num_gifts, int * gift_ids ) {
  int i;
  double distance = NP_DISTANCE_TO_GIFT( gift_ids[num_gifts-1] );

  for ( i = num_gifts - 2; i >= 0; i-- ) {
    distance += distance_between_gifts( gift_ids[i], gift_ids[i+1] );
  }

  return distance + NP_DISTANCE_TO_GIFT( gift_ids[0] );
}

double trip_with_insert_cost( int num_gifts, int * gift_ids, int new_gift_id, int insertion_point ) {
  int i;
  int * gift_ids_plus = ALLOC_N( int, 1000 );
  for ( i = 0; i < insertion_point; i++ ) {
    gift_ids_plus[ i ] =  gift_ids[ i ];
  }
  gift_ids_plus[ insertion_point ] = new_gift_id;
  for ( i = insertion_point; i < num_gifts; i++ ) {
    gift_ids_plus[ i + 1 ] =  gift_ids[ i ];
  }
  double cost = trip_cost( num_gifts + 1, gift_ids_plus );

  xfree( gift_ids_plus );

  return cost;
}

static double distance_to[1000];

// This is called a *LOT* and would be good to optimise
int trip_lowest_cost_insert( int num_gifts, int * gift_ids, int new_gift_id ) {
  int i;
  if ( num_gifts < 1 ) {
    return 0;
  }

  distance_to[0] = NP_DISTANCE_TO_GIFT( gift_ids[0] );
  for ( i = 1; i < num_gifts; i++ ) {
    distance_to[i] = distance_between_gifts( gift_ids[i-1], gift_ids[i] ) + distance_to[i-1];
  }

  double current_weight = 10.0;

  double extra_weight = WEIGHT_OF_GIFT( new_gift_id );
  double extra_cost;
  double extra_distance, previous_extra_distance;
  double best_extra_cost;
  int best_insert_id;

  // This is extra cost of insert at end (position num_gifts)
  previous_extra_distance = distance_between_gifts( new_gift_id, gift_ids[num_gifts-1] );
  best_extra_cost = ( current_weight * NP_DISTANCE_TO_GIFT( new_gift_id ) )
                    + ( ( current_weight + extra_weight ) * previous_extra_distance )
                    + ( extra_weight * distance_to[num_gifts-1] )
                    - ( current_weight * NP_DISTANCE_TO_GIFT( gift_ids[num_gifts-1] ) );
  best_insert_id = num_gifts;

  current_weight += WEIGHT_OF_GIFT( gift_ids[num_gifts-1] );

  for ( i = num_gifts - 1; i > 0; i-- ) {
    extra_distance = distance_between_gifts( new_gift_id, gift_ids[i-1] );
    extra_cost = current_weight * (
                    previous_extra_distance +
                    extra_distance - distance_to[i] + distance_to[i-1] )
                 + extra_weight * (
                    extra_distance + distance_to[i-1]
                 );

    if ( extra_cost < best_extra_cost ) {
      best_extra_cost = extra_cost;
      best_insert_id = i;
    }

    previous_extra_distance = extra_distance;
    current_weight += WEIGHT_OF_GIFT( gift_ids[i-1] );
  }

  // This is extra cost of insert at start (position 0)
  extra_cost = ( current_weight * previous_extra_distance )
                + ( ( current_weight + extra_weight ) * NP_DISTANCE_TO_GIFT( new_gift_id ) )
                - ( current_weight * NP_DISTANCE_TO_GIFT( gift_ids[0] ) );

  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = 0;
  }

  return best_insert_id;
}

int trip_lowest_cost_insert2( int num_gifts, int * gift_ids, int new_gift_id, double * save_cost ) {
  int i;
  if ( num_gifts < 1 ) {
    *save_cost =  SOLO_SCORE_OF_GIFT( new_gift_id );
    return 0;
  }

  distance_to[0] = NP_DISTANCE_TO_GIFT( gift_ids[0] );
  for ( i = 1; i < num_gifts; i++ ) {
    distance_to[i] = distance_between_gifts( gift_ids[i-1], gift_ids[i] ) + distance_to[i-1];
  }

  double current_weight = 10.0;

  double extra_weight = WEIGHT_OF_GIFT( new_gift_id );
  double extra_cost;
  double extra_distance, previous_extra_distance;
  double best_extra_cost;
  int best_insert_id;

  // This is extra cost of insert at end (position num_gifts)
  previous_extra_distance = distance_between_gifts( new_gift_id, gift_ids[num_gifts-1] );
  best_extra_cost = ( current_weight * NP_DISTANCE_TO_GIFT( new_gift_id ) )
                    + ( ( current_weight + extra_weight ) * previous_extra_distance )
                    + ( extra_weight * distance_to[num_gifts-1] )
                    - ( current_weight * NP_DISTANCE_TO_GIFT( gift_ids[num_gifts-1] ) );
  best_insert_id = num_gifts;

  current_weight += WEIGHT_OF_GIFT( gift_ids[num_gifts-1] );

  for ( i = num_gifts - 1; i > 0; i-- ) {
    extra_distance = distance_between_gifts( new_gift_id, gift_ids[i-1] );
    extra_cost = current_weight * (
                    previous_extra_distance +
                    extra_distance - distance_to[i] + distance_to[i-1] )
                 + extra_weight * (
                    extra_distance + distance_to[i-1]
                 );

    if ( extra_cost < best_extra_cost ) {
      best_extra_cost = extra_cost;
      best_insert_id = i;
    }

    previous_extra_distance = extra_distance;
    current_weight += WEIGHT_OF_GIFT( gift_ids[i-1] );
  }

  // This is extra cost of insert at start (position 0)
  extra_cost = ( current_weight * previous_extra_distance )
                + ( ( current_weight + extra_weight ) * NP_DISTANCE_TO_GIFT( new_gift_id ) )
                - ( current_weight * NP_DISTANCE_TO_GIFT( gift_ids[0] ) );

  if ( extra_cost < best_extra_cost ) {
    best_extra_cost = extra_cost;
    best_insert_id = 0;
  }

  *save_cost = best_extra_cost;

  return best_insert_id;
}


void insert_gift_into_trip_at( int current_trip_size, int * trip_gift_ids, int new_gift_id, int insert_at ) {
  int i;
  int last_moved_id, tmp;

  last_moved_id = trip_gift_ids[insert_at];
  trip_gift_ids[insert_at] = new_gift_id;

  for ( i = insert_at + 1; i <= current_trip_size; i++ ) {
    tmp = last_moved_id;
    last_moved_id = trip_gift_ids[ i ];
    trip_gift_ids[ i ] = tmp;
  }

  return;
}

void draw_max_n_from( int num_items, int max_drawn, int * source, int * num_drawn, int * draw_buffer, int * num_remaining, int * remaining_buffer ) {
  int i, j, k, a, skip, ndrawn = 0, nrem = 0;

  if (num_items == 0) {
    *num_drawn = 0;
    *num_remaining = 0;
    return;
  }

  for ( j = 0; j < max_drawn; j++ ) {
    skip = 0;
    a = genrand_int31() % num_items;
    i = source[a];
    // Only accept unique values
    for ( k = 0; k < ndrawn; k++ ) {
      if ( draw_buffer[ k ] == i ) {
        skip = 1;
        break;
      }
    }
    if (skip) { continue; }
    draw_buffer[ ndrawn ] = i;
    ndrawn++;
  }
  *num_drawn = ndrawn;

  for ( j = 0; j < num_items; j++ ) {
    skip = 0;
    i = source[j];
    for ( k = 0; k < ndrawn; k++ ) {
      if ( draw_buffer[ k ] == i ) {
        skip = 1;
      }
    }
    if (skip) { continue; }
    remaining_buffer[nrem] = i;
    nrem++;
  }
  *num_remaining = nrem;

  return;
}

// A Fisher-Yates shuffle
void shuffle_ints( int n, int *array ) {
  int i, tmp, r;
  for ( i = n-1; i >= 0; i-- ) {
    // This will be slightly biased for large n, but it is not a
    // noticeable issue for dynamic solutions
    r = genrand_int31() % ( i + 1 );
    tmp = array[r];
    array[r] = array[i];
    array[i] = tmp;
  }
  return;
}

double dot_product_sum( int num_items, double * vec_a, double * vec_b ) {
  int i;
  double t = 0.0;
  for ( i = 0; i < num_items; i++ ) {
    // TODO: This could be optimised with SIMD:
    t += vec_a[i] * vec_b[i];
  }
  return t;
}

// Probably makes no difference - we need __m256d really, but my laptop does not have it
static double sum_v[2];
static __m128d simd_x, simd_y, simd_t;

double fast_dot_product_sum( int num_items, double * vec_a, double * vec_b ) {
  int i;
  int items_aligned = 2 * (num_items/2);
  double t = 0.0;
  simd_t = _mm_setzero_pd();

  for ( i = 0; i < items_aligned; i += 2 ) {
    simd_x = _mm_load_pd( vec_a + i );
    simd_y = _mm_load_pd( vec_b + i );
    simd_x = _mm_mul_pd( simd_x, simd_y );
    simd_t = _mm_add_pd( simd_x, simd_t );
  }
  _mm_store_pd( sum_v, simd_t );

  for ( i = items_aligned; i < num_items; i++ ) {
    t += vec_a[i] * vec_b[i];
  }

  return t + sum_v[0] + sum_v[1];
}

static int trips_gifts[100000];
static int gift_cache_id[100000];
static int gdcache_size = 0;
static double * gifts_distance_cache = NULL;

void gdcache__new_cache_from_trips( int num_trips, Trip ** trips) {
  int i,j,n;
  Trip * trip;
  n = 0;

  for( i = 0; i < num_trips; i++ ) {
    trip = trips[i];
    for( j = 0; j < trip->num_gifts; j++ ) {
      trips_gifts[n] = trip->gifts[j];
      n++;
    }
  }

  gdcache__new_cache( n, trips_gifts );
  return;
}

void gdcache__new_cache( int num_gifts, int * gift_ids ) {
  int i,j;
  xfree( gifts_distance_cache );
  gifts_distance_cache = ALLOC_N( double, num_gifts * num_gifts );
  gdcache_size = num_gifts;
  for( i = 0; i < num_gifts; i++ ) {
    gift_cache_id[ gift_ids[i] ] = i;
    for( j = 0; j < num_gifts; j++ ) {
      if ( i == j ) {
        gifts_distance_cache[ i * num_gifts + j ] = 0.0;
      } else {
        gifts_distance_cache[ i * num_gifts + j ] = distance_between_gifts( gift_ids[i], gift_ids[j] );
      }
    }
  }
  return;
}

double gdcache__distance_between_gifts( int gift_a_id, int gift_b_id ) {
  return gifts_distance_cache[ gift_cache_id[gift_a_id] * gdcache_size + gift_cache_id[gift_b_id] ];
}
