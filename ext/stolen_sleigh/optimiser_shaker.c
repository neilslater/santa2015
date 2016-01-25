// ext/stolen_sleigh/optimiser_shaker.c

#include "optimiser_shaker.h"

// Implements an optimiser based on re-trying optimal gift positions in trips, in different orders

inline double optimiser_shaker__probability( double delta, double inv_t ) {
  return exp( -delta * inv_t );
}

static Trip ** tmp_trips;
static int * gift_pool;

int optimiser_shaker__multi_trip_sample( int num_trips, Trip ** src_trips, int max_sample ) {
  int i, total_drawn = 0;
  Trip * src_trip, * remaining_trip;
  for ( i = 0; i < num_trips; i++ ) {
    src_trip = src_trips[i];
    remaining_trip = tmp_trips[i];
    total_drawn += trip__sample_gifts_var( src_trip, max_sample, remaining_trip, ( gift_pool + total_drawn ) );
  }
  return total_drawn;
}

int optimiser_shaker__multi_trip_sample_fixed( int num_trips, Trip ** src_trips, int max_sample ) {
  int i, total_drawn = 0;
  Trip * src_trip, * remaining_trip;
  for ( i = 0; i < num_trips; i++ ) {
    src_trip = src_trips[i];
    remaining_trip = tmp_trips[i];
    total_drawn += trip__sample_gifts( src_trip, max_sample, remaining_trip, ( gift_pool + total_drawn ) );
  }
  return total_drawn;
}

static int random_trip_ids[200];

// This samples only a few items, one at a time
int optimiser_shaker__multi_trip_sample_limit( int num_trips, Trip ** src_trips, int max_sample ) {
  int i, total_drawn = 0, trip_id;
  Trip * src_trip, * remaining_trip;

  shuffle_ints( num_trips, random_trip_ids );

  for ( i = 0; i < max_sample; i++ ) {
    trip_id = random_trip_ids[i];
    src_trip = src_trips[trip_id];
    remaining_trip = tmp_trips[trip_id];
    total_drawn += trip__sample_gifts( src_trip, 1, remaining_trip, ( gift_pool + total_drawn ) );
  }

  for ( i = max_sample; i < num_trips; i++ ) {
    trip_id = random_trip_ids[i];
    src_trip = src_trips[trip_id];
    remaining_trip = tmp_trips[trip_id];
    trip__quick_copy( remaining_trip, src_trip );
  }

  return total_drawn;
}

double optimiser_shaker__multi_trip_cost( int num_trips, Trip ** trips ) {
  double score = 0.0;
  int i;
  for ( i = 0; i < num_trips; i++ ) {
    score += trip__score( trips[i] );
  }
  return score;
}

int optimiser_shaker__insert_gift_at_lowest_cost( int num_trips, int gift_id ) {
  int i, best_p, p;
  Trip *trip;
  int best_trip_id = -1;
  double best_cost = 1000000000.0;
  double weight_to_insert = WEIGHT_OF_GIFT(gift_id);


  for ( i = 0; i < num_trips; i++ ) {
    trip = tmp_trips[i];

    if ( weight_to_insert + trip__weight( trip ) > 1000.0 || trip->num_gifts > 149 ) {
      continue;
    }

    p = trip__lowest_cost_insert_at_gdcache( trip, gift_id, &best_cost );
    if ( p >= 0 ) {
      best_p = p;
      best_trip_id = i;
    }
  }

  if ( best_trip_id > -1 ) {
    trip__add_gift_gdcache( tmp_trips[best_trip_id], best_p, gift_id );
    return 1;
  }

  return 0;
}

int optimiser_shaker__insert_gift_at_lowest_cost_randomly( int num_trips, int gift_id ) {
  int i, best_p, p, n = 0;
  Trip *trip;
  int best_trip_id = -1;
  double best_cost = 1000000000.0;
  double weight_to_insert = WEIGHT_OF_GIFT(gift_id);

  while ( best_trip_id < 0 ) {
    i = genrand_int31() % num_trips;
    trip = tmp_trips[i];

    if ( weight_to_insert + trip__weight( trip ) > 1000.0 || trip->num_gifts > 149 ) {
      continue;
    }

    p = trip__lowest_cost_insert_at_gdcache( trip, gift_id, &best_cost );
    if ( p >= 0 ) {
      best_p = p;
      best_trip_id = i;
    }

    if ( n++ > 3 ) {
        break;
    }
  }

  if ( best_trip_id > -1 ) {
    trip__add_gift_gdcache( tmp_trips[best_trip_id], best_p, gift_id );
    return 1;
  }

  return 0;
}

void optimiser_shaker__save_scratch_results( int num_trips, Trip ** src_trips ) {
  int i;
  for ( i = 0; i < num_trips; i++ ) {
    trip__quick_copy( src_trips[i], tmp_trips[i] );
  }
  return;
}

double optimiser_shaker__multi_trip_shake( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws ) {
  double best_score = optimiser_shaker__multi_trip_cost( num_trips, src_trips );
  int i, j, a, k, ng, gift_id, skip, p, ok, gift_pool_length;

  gdcache__new_cache_from_trips( num_trips, src_trips );

  double this_score;
  if ( max_trip_draw > MAX_MULTI_DRAW ) {
    max_trip_draw = MAX_MULTI_DRAW;
  }

  for ( i = 0; i < num_draws; i++ ) {
    // NOT sure if _fixed version is better here?
    gift_pool_length = optimiser_shaker__multi_trip_sample( num_trips, src_trips, max_trip_draw );

    shuffle_ints( gift_pool_length, gift_pool );

    skip = 0;
    for ( j = 0; j < gift_pool_length; j++ ) {
      ok = optimiser_shaker__insert_gift_at_lowest_cost( num_trips, gift_pool[j] );
      if ( ! ok ) {
        skip = 1;
        break;
      }
    }
    if ( skip ) {
      // The gifts didn't fit back in same space, try another set
      continue;
    }

    this_score = optimiser_shaker__multi_trip_cost( num_trips, tmp_trips );

    if ( this_score < best_score ) {
      best_score = this_score;
      optimiser_shaker__save_scratch_results( num_trips, src_trips );
    }
  }

  return best_score;
}


double optimiser_shaker__multi_trip_shake_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double inv_t, double t_factor ) {
  double current_score = optimiser_shaker__multi_trip_cost( num_trips, src_trips );
  int i, j, a, k, ng, gift_id, skip, p, ok, gift_pool_length;

  gdcache__new_cache_from_trips( num_trips, src_trips );

  double this_score;
  if ( max_trip_draw > MAX_MULTI_DRAW ) {
    max_trip_draw = MAX_MULTI_DRAW;
  }

  for ( i = 0; i < num_draws; i++ ) {
    gift_pool_length = optimiser_shaker__multi_trip_sample( num_trips, src_trips, max_trip_draw );

    shuffle_ints( gift_pool_length, gift_pool );

    skip = 0;
    for ( j = 0; j < gift_pool_length; j++ ) {
      ok = optimiser_shaker__insert_gift_at_lowest_cost( num_trips, gift_pool[j] );
      if ( ! ok ) {
        skip = 1;
        break;
      }
    }
    if ( skip ) {
      // The gifts didn't fit back in same space, try another set
      continue;
    }

    this_score = optimiser_shaker__multi_trip_cost( num_trips, tmp_trips );

    if ( this_score < current_score || optimiser_shaker__probability( this_score - current_score, inv_t ) > genrand_real1() ) {
      current_score = this_score;
      optimiser_shaker__save_scratch_results( num_trips, src_trips );
    }

    inv_t *= t_factor;
  }

  return current_score;
}

// Simple shake makes more limited selections and placements
double optimiser_shaker__simple_shake_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double inv_t, double t_factor ) {
  double current_score = optimiser_shaker__multi_trip_cost( num_trips, src_trips );
  int i, j, a, k, ng, gift_id, skip, p, ok, gift_pool_length;

  gdcache__new_cache_from_trips( num_trips, src_trips );

  for ( i = 0; i < num_trips; i++ ) {
    random_trip_ids[i] = i;
  }

  double this_score;
  if ( max_trip_draw > num_trips ) {
    max_trip_draw = num_trips;
  }

  for ( i = 0; i < num_draws; i++ ) {
    gift_pool_length = optimiser_shaker__multi_trip_sample_limit( num_trips, src_trips, ( 1 + ( genrand_int31() % max_trip_draw ) ) );

    skip = 0;
    for ( j = 0; j < gift_pool_length; j++ ) {
      ok = optimiser_shaker__insert_gift_at_lowest_cost_randomly( num_trips, gift_pool[j] );
      if ( ! ok ) {
        skip = 1;
        break;
      }
    }
    if ( skip ) {
      // The gifts didn't fit back in same space, try another set
      inv_t *= t_factor;
      continue;
    }

    this_score = optimiser_shaker__multi_trip_cost( num_trips, tmp_trips );

    if ( this_score < current_score || optimiser_shaker__probability( this_score - current_score, inv_t ) > genrand_real1() ) {
      current_score = this_score;
      optimiser_shaker__save_scratch_results( num_trips, src_trips );
    }

    inv_t *= t_factor;
  }

  return current_score;
}

void optimiser_shaker__init_buffers() {
  int i;
  tmp_trips = ALLOC_N( Trip*, MAX_MULTI_TRIPS );
  gift_pool = ALLOC_N( int, 5000 );
  for ( i = 0; i < MAX_MULTI_TRIPS; i++ ) {
    tmp_trips[i] = trip__create();
    trip__init( tmp_trips[i], 0 );
  }
}

Trip ** optimiser_shaker__get_tmp_trips() { return tmp_trips; }
int * optimiser_shaker__get_gift_pool() { return gift_pool; }
