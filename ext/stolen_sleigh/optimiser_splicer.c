// ext/stolen_sleigh/optimiser_splicer.c

#include "optimiser_splicer.h"

// Implements an optimiser based on splitting trips up. This is incomplete.

inline double optimiser_splicer__probability( double delta, double inv_t ) {
  return exp( -delta * inv_t );
}

inline double optimiser_splicer__multi_trip_cost( int num_trips, Trip ** trips ) {
  double score = 0.0;
  int i;
  for ( i = 0; i < num_trips; i++ ) {
    score += trip__score( trips[i] );
  }
  return score;
}

inline double optimiser_splicer__score_move( Trip ** trips, int src_trip_id, int src_idx, int num_draw, int dst_trip_id, int dst_idx ) {
  double score;

  if ( src_trip_id == dst_trip_id ) {
    score = trip__splice_self_score( trips[src_trip_id], src_idx, num_draw, dst_idx ) - trip__score( trips[src_trip_id] );
  } else {
    score = trip__splice_from_a_to_b_score( trips[src_trip_id], src_idx, num_draw, trips[dst_trip_id], dst_idx )
            - trip__score( trips[src_trip_id] ) - trip__score( trips[dst_trip_id] );
  }

  return score;
}

inline void optimiser_splicer__apply_move( Trip ** trips, int src_trip_id, int src_idx, int num_draw, int dst_trip_id, int dst_idx ) {
  if ( src_trip_id == dst_trip_id ) {
    trip__splice_self( trips[src_trip_id], src_idx, num_draw, dst_idx );
  } else {
    trip__splice_from_a_to_b( trips[src_trip_id], src_idx, num_draw, trips[dst_trip_id], dst_idx );
  }
  return;
}

double optimiser_splicer__multi_trip_splice_sa( int num_trips, Trip ** src_trips, int max_trip_draw, int num_draws, double inv_t, double t_factor ) {
  double current_score = optimiser_splicer__multi_trip_cost( num_trips, src_trips );
  double test_score;
  int i, src_trip_id, src_idx, num_draw, dst_trip_id, dst_idx, min_dst_idx, max_dst_idx;
  gdcache__new_cache_from_trips( num_trips, src_trips );

  double delta_score;
  if ( max_trip_draw > MAX_MULTI_DRAW ) {
    max_trip_draw = MAX_MULTI_DRAW;
  }

  for ( i = 0; i < num_draws; i++ ) {
    // Generate move
    src_trip_id = genrand_int31() % num_trips;
    if ( src_trips[src_trip_id]->num_gifts ) {
        src_idx = genrand_int31() % src_trips[src_trip_id]->num_gifts;
    } else {
        inv_t *= t_factor;
        continue;
    }
    num_draw = 1 + ( genrand_int31() % max_trip_draw );
    dst_trip_id = genrand_int31() % num_trips;

    if ( num_draw + src_idx > src_trips[src_trip_id]->num_gifts ) {
       num_draw =  src_trips[src_trip_id]->num_gifts - src_idx;
    }

    /*
    min_dst_idx = src_idx - 20;
    if (min_dst_idx < 0) {
        min_dst_idx = 0;
    }
    max_dst_idx = src_idx + 20;

    if ( dst_trip_id == src_trip_id ) {
        if ( max_dst_idx > ( src_trips[dst_trip_id]->num_gifts - num_draw ) ) {
            max_dst_idx = src_trips[dst_trip_id]->num_gifts - num_draw;
        }
    } else {
        if ( max_dst_idx > src_trips[dst_trip_id]->num_gifts ) {
            max_dst_idx = src_trips[dst_trip_id]->num_gifts;
        }
    }
    if ( min_dst_idx >= max_dst_idx ) {
        min_dst_idx = 0;
    }

    dst_idx = min_dst_idx + ( genrand_int31() % ( 1 + max_dst_idx - min_dst_idx) );
    */
    if ( dst_trip_id == src_trip_id ) {
        dst_idx = genrand_int31() % ( 1 + src_trips[dst_trip_id]->num_gifts - num_draw );
    } else {
        dst_idx = genrand_int31() % ( 1 + src_trips[dst_trip_id]->num_gifts );
    }


    // Score move
    delta_score = optimiser_splicer__score_move( src_trips, src_trip_id, src_idx, num_draw, dst_trip_id, dst_idx );
    inv_t *= t_factor;

    if ( delta_score > 100000000.0 ) {
        continue;
    }

    if ( delta_score < 0.0 || optimiser_splicer__probability( delta_score, inv_t ) > genrand_real1() ) {
      // Apply move
      current_score += delta_score;
      optimiser_splicer__apply_move( src_trips, src_trip_id, src_idx, num_draw, dst_trip_id, dst_idx );

      /*
      test_score = optimiser_splicer__multi_trip_cost( num_trips, src_trips );
      if ( fabs( test_score - current_score ) > 1.0 ) {
        printf( "Move %d. Score diverged. Measured score %f, but deltas went to %f\n", i, test_score, current_score );
        printf( "   src_trip_id %d, src_idx %d, num_draw %d, dst_trip_id %d, dst_idx %d\n", src_trip_id, src_idx, num_draw, dst_trip_id, dst_idx );
      } */
    }
  }

  return current_score;
}
