// ext/stolen_sleigh/ruby_class_trip.c

#include "ruby_class_trip.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Ruby bindings for training data arrays - the deeper implementation is in
//  struct_trip.c
//

inline VALUE trip_as_ruby_class( Trip *trip , VALUE klass ) {
  return Data_Wrap_Struct( klass, trip__gc_mark, trip__destroy, trip );
}

VALUE trip_alloc(VALUE klass) {
  return trip_as_ruby_class( trip__create(), klass );
}

Trip *get_trip_struct( VALUE obj ) {
  Trip *trip;
  Data_Get_Struct( obj, Trip, trip );
  return trip;
}

void assert_value_wraps_trip( VALUE obj ) {
  if ( TYPE(obj) != T_DATA ||
      RDATA(obj)->dfree != (RUBY_DATA_FUNC)trip__destroy) {
    rb_raise( rb_eTypeError, "Expected a Trip object, but got something else" );
  }
}

/* Document-class: StolenSleigh::Trip
 * This class models single trips.
 *
 * An instance of this class represents a single journey starting from the North Pole, visiting the
 * location of an array of gifts in order, and returning to the North Pole.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  StolenSleigh::Trip method definitions
//

/* @overload initialize( num_gifts )
 * Creates a new StolenSleigh::Trip
 * @param [Integer] num_gifts number of starting elements in the gifts array
 * @return [StolenSleigh::Trip] new instance
 */
VALUE trip_rbobject__initialize( VALUE self, VALUE rv_num_gifts ) {
  Trip *trip = get_trip_struct( self );

  trip__init( trip, NUM2INT( rv_num_gifts ) );

  return self;
}

/* @overload clone
 * When cloned, the returned Trip has deep copies of all data.
 * @return [StolenSleigh::Trip] copy
 */
VALUE trip_rbobject__initialize_copy( VALUE copy, VALUE orig ) {
  Trip *trip_copy;
  Trip *trip_orig;

  if (copy == orig) return copy;
  trip_orig = get_trip_struct( orig );
  trip_copy = get_trip_struct( copy );

  trip__deep_copy( trip_copy, trip_orig );

  return copy;
}

/* @!attribute [r] gifts
 * A copy of the array of gifts. Alterations to the array do not affect the trip.
 * @return [NArray<int>]
 */
VALUE trip_rbobject__get_narr_gifts( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  struct NARRAY *narr;
  int shape[1];
  int i, * narr_gifts_ptr;

  shape[0] = trip->num_gifts;
  volatile VALUE rv_narr_gifts = na_make_object( NA_LINT, 1, shape, cNArray );

  GetNArray( rv_narr_gifts, narr );
  narr_gifts_ptr = (int*) narr->ptr;
  for( i = 0; i < trip->num_gifts; i++ ) {
    narr_gifts_ptr[i] = trip->gifts[i];
  }

  return rv_narr_gifts;
}

/* @!attribute [r] num_gifts
 * The number of gifts in the trip.
 * @return [Integer]
 */
VALUE trip_rbobject__get_num_gifts( VALUE self ) {
  return INT2NUM( get_trip_struct( self )->num_gifts );
}

/* @overload from_narray( gift_ids )
 * Creates new Trip object from supplied NArray of gift_ids.
 * @param [NArray<int>] gift_ids gifts in order of visit
 * @return [StolenSleigh::Trip] new instance
 */
VALUE trip_rbclass__from_narray( VALUE self, VALUE rv_narray ) {
  Trip *trip;

  struct NARRAY *narr;
  volatile VALUE new_narray = na_cast_object(rv_narray, NA_LINT);
  GetNArray( new_narray, narr );
  if ( narr->rank != 1 ) {
    rb_raise( rb_eArgError, "Input gifts array has wrong rank %d", narr->rank );
  }
  if ( narr->shape[0] > 150 ) {
    rb_raise( rb_eArgError, "Input gifts array is too large (%d elements)", narr->shape[0] );
  }
  // Maybe check for repeated gift ids or total weight here as well?

  VALUE rv_trip = trip_alloc( StolenSleigh_Trip );
  trip = get_trip_struct( rv_trip );

  trip__set_gifts( trip, new_narray );

  return rv_trip;
}

/* @overload score( input )
 * Total score associated with this trip.
 * @return [Float]
 */
VALUE trip_rbobject__score( VALUE self ) {
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  return DBL2NUM( trip__score( get_trip_struct( self ) ) );
}

/* @overload gifts_weight( )
 * Total weight of all gifts in the trip. The maximum allowed total weight is 1000.0
 * @return [Float]
 */
VALUE trip_rbobject__gifts_weight( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  return DBL2NUM( trip__weight( trip ) );
}

/* @overload efficiency( )
 * Metric for how well the trip improves the score compared to score of individual trips for
 * each gift. Higher is better.
 * @return [Float]
 */
VALUE trip_rbobject__efficiency( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  return DBL2NUM( trip__efficiency( trip ) );
}

/* @overload distance( )
 * Total distance covered by the trip.
 * @return [Float]
 */
VALUE trip_rbobject__distance( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  return DBL2NUM( trip__distance( trip ) );
}

/* @overload total_tension( )
 * Measure of how much potential improvement is possible if there were no diversions in the trip.
 * @return [Float]
 */
VALUE trip_rbobject__total_tension( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  return DBL2NUM( trip__total_tension( trip ) );
}

/* @overload lowest_cost_insert_at( new_gift_id )
 * Calculates the best possible insertion point of a gift into the trip.
 * @param [Integer] new_gift_id identifier for gift to add
 * @return [Array<Integer,Float>] index of best position and cost of adding gift at that position
 */
VALUE trip_rbobject__lowest_cost_insert_at( VALUE self, VALUE rv_new_gift_id ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  double cost = 1000000000.0;
  int pos = trip__lowest_cost_insert_at( trip, NUM2INT( rv_new_gift_id ), &cost );

  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, INT2NUM( pos ) );
  rb_ary_store( rv_result, 1, DBL2NUM( cost ) );
  return rv_result;
}

/* @overload lowest_cost_break_at( )
 * Calculates lowest cost break point if trip is broken in two.
 * @return Array<Integer,Float>] index of best break point and cost of breaking up the trip
 */
VALUE trip_rbobject__lowest_cost_break_at( VALUE self ) {
  Trip *trip = get_trip_struct( self );
  if ( ! gifts ) {
    rb_raise( rb_eRuntimeError, "Gifts array not setup" );
  }
  double cost = 1000000000.0;
  int pos = trip__lowest_cost_break_at( trip, &cost );

  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, INT2NUM( pos ) );
  rb_ary_store( rv_result, 1, DBL2NUM( cost ) );
  return rv_result;
}

/* @overload set_gift( idx, gift_id )
 * Sets gift id
 * @return [Integer] gift_id
 */
VALUE trip_rbobject__set_gift( VALUE self, VALUE rv_idx, VALUE rv_gift_id ) {
  Trip *trip = get_trip_struct( self );
  int idx, gift_id;
  idx = NUM2INT( rv_idx );
  if ( idx < 0 || idx >= trip->num_gifts ) {
    rb_raise( rb_eArgError, "Index %d out of bounds for trip [0..%d]", idx, trip->num_gifts - 1 );
  }

  gift_id = NUM2INT( rv_gift_id );
  if ( gift_id  < 0 || gift_id  > 99999 ) {
    rb_raise( rb_eArgError, "Gift id %d out of bounds [0..99999]", gift_id );
  }

  trip__set_gift( trip, idx, gift_id );

  return rv_gift_id;
}


/* @overload add_gift( idx, gift_id )
 * Inserts gift at chosen index position
 * @return [Integer] gift_id
 */
VALUE trip_rbobject__add_gift( VALUE self, VALUE rv_idx, VALUE rv_gift_id ) {
  Trip *trip = get_trip_struct( self );
  int idx, gift_id;
  idx = NUM2INT( rv_idx );
  if ( idx < 0 || idx > trip->num_gifts ) {
    rb_raise( rb_eArgError, "Index %d out of bounds for trip [0..%d]", idx, trip->num_gifts );
  }

  gift_id = NUM2INT( rv_gift_id );
  if ( gift_id  < 0 || gift_id  > 99999 ) {
    rb_raise( rb_eArgError, "Gift id %d out of bounds [0..99999]", gift_id );
  }

  trip__add_gift( trip, idx, gift_id );

  return rv_gift_id;
}


/* @overload get_gift( idx )
 * Returns gift_id at supplied position
 * @param [Integer] idx index of position
 * @return [Integer] gift_id
 */
VALUE trip_rbobject__get_gift( VALUE self, VALUE rv_idx ) {
  Trip *trip = get_trip_struct( self );
  int idx;
  idx = NUM2INT( rv_idx );
  if ( idx < 0 || idx >= trip->num_gifts ) {
    rb_raise( rb_eArgError, "Index %d out of bounds for trip [0..%d]", idx, trip->num_gifts - 1 );
  }

  return INT2NUM( trip->gifts[ idx ] );
}

/* @overload remove_gifts_by_idx( idxs )
 * Removes gifts at chosen indexes from trip
 * @param [NArray<int>] idxs index of positions to remove
 * @return [StolenSleigh::Trip] self
 */
VALUE trip_rbobject__remove_gifts_by_idx( VALUE self, VALUE rv_narray ) {
  Trip *trip = get_trip_struct( self );

  struct NARRAY *narr;
  volatile VALUE new_narray = na_cast_object(rv_narray, NA_LINT);
  GetNArray( new_narray, narr );
  if ( narr->rank != 1 ) {
    rb_raise( rb_eArgError, "Input gifts array has wrong rank %d", narr->rank );
  }
  if ( narr->shape[0] > 150 ) {
    rb_raise( rb_eArgError, "Input gifts array is too large (%d elements)", narr->shape[0] );
  }
  // Maybe check for repeated gift ids or total weight here as well?

  trip__remove_gifts_by_idx( trip, narr->shape[0], (int*) narr->ptr );

  return self;
}

/* @overload remove_gifts_by_gift_id( gift_ids )
 * Removes gifts with chosen gift_ids from trip
 * @param [NArray<int>] gift_ids identifiers of gifts to remove
 * @return [StolenSleigh::Trip] self
 */
VALUE trip_rbobject__remove_gifts_by_gift_id( VALUE self, VALUE rv_narray ) {
  Trip *trip = get_trip_struct( self );

  struct NARRAY *narr;
  volatile VALUE new_narray = na_cast_object(rv_narray, NA_LINT);
  GetNArray( new_narray, narr );
  if ( narr->rank != 1 ) {
    rb_raise( rb_eArgError, "Input gifts array has wrong rank %d", narr->rank );
  }
  if ( narr->shape[0] > 150 ) {
    rb_raise( rb_eArgError, "Input gifts array is too large (%d elements)", narr->shape[0] );
  }
  // Maybe check for repeated gift ids or total weight here as well?

  trip__remove_gifts_by_gift_id( trip, narr->shape[0], (int*) narr->ptr );

  return self;
}


/* @overload sample_gifts( max_sample, remaining_trip )
 * Samples gifts randomly from trip, and alters supplied trip to match what would be left.
 * @param [Integer] max_sample the maximum number of items to sample
 * @param [StolenSleigh::Trip] remaining_trip a trip object that is over-written with a copy of the starting trip with sampled gifts removed
 * @return [NArray<int>] sampled gift ids
 */
VALUE trip_rbobject__sample_gifts( VALUE self, VALUE rv_max_sample, VALUE rv_trip_remaining ) {
  Trip *trip = get_trip_struct( self );
  assert_value_wraps_trip( rv_trip_remaining );
  Trip *trip_remaining = get_trip_struct( rv_trip_remaining );
  int max_sample = NUM2INT( rv_max_sample );
  int *sample = ALLOC_N( int, 150 );
  int sample_size = trip__sample_gifts( trip, max_sample, trip_remaining, sample );

  volatile VALUE rv_result = na_make_object( NA_LINT, 1, &sample_size, cNArray );
  struct NARRAY *narr;
  GetNArray( rv_result, narr );
  memcpy( (int*) narr->ptr, sample, sample_size * sizeof(int) );
  xfree( sample );

  return rv_result;
}

/* @overload sample_gifts_var( max_sample, remaining_trip )
 * Samples gifts randomly from trip, and alters supplied trip to match what would be left. The max_sample
 * parameter is used differently from sample_gifts, and this method has more random variation in the number of
 * gifts sampled.
 * @param [Integer] max_sample the maximum number of items to sample
 * @param [StolenSleigh::Trip] remaining_trip a trip object that is over-written with a copy of the starting trip with sampled gifts removed
 * @return [NArray<int>] sampled gift ids
 */
VALUE trip_rbobject__sample_gifts_var( VALUE self, VALUE rv_max_sample, VALUE rv_trip_remaining ) {
  Trip *trip = get_trip_struct( self );
  assert_value_wraps_trip( rv_trip_remaining );
  Trip *trip_remaining = get_trip_struct( rv_trip_remaining );
  int max_sample = NUM2INT( rv_max_sample );
  int *sample = ALLOC_N( int, 150 );
  int sample_size = trip__sample_gifts_var( trip, max_sample, trip_remaining, sample );

  volatile VALUE rv_result = na_make_object( NA_LINT, 1, &sample_size, cNArray );
  struct NARRAY *narr;
  GetNArray( rv_result, narr );
  memcpy( (int*) narr->ptr, sample, sample_size * sizeof(int) );
  xfree( sample );

  return rv_result;
}

/* @overload splice( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__splice( VALUE self, VALUE rv_start_idx, VALUE rv_num_splice_out, VALUE rv_input_trip, VALUE rv_output_trip ) {
  Trip *trip = get_trip_struct( self );

  assert_value_wraps_trip( rv_input_trip );
  assert_value_wraps_trip( rv_output_trip );
  Trip *input_trip = get_trip_struct( rv_input_trip );
  Trip *output_trip = get_trip_struct( rv_output_trip );

  int start_idx = NUM2INT( rv_start_idx );
  int num_splice_out = NUM2INT( rv_num_splice_out );

  trip__splice( trip, start_idx, num_splice_out, input_trip, output_trip );

  return rv_output_trip;
}

/* @overload splice( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__cross_splice( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_idx_in_a, VALUE rv_trip_b, VALUE rv_idx_out_b, VALUE rv_num_b, VALUE rv_idx_in_b ) {
  Trip *trip_a = get_trip_struct( self );

  assert_value_wraps_trip( rv_trip_b );
  Trip *trip_b = get_trip_struct( rv_trip_b );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_a = NUM2INT( rv_idx_in_a );
  int idx_out_b = NUM2INT( rv_idx_out_b );
  int num_b = NUM2INT( rv_num_b );
  int idx_in_b = NUM2INT( rv_idx_in_b );

  trip__mutual_splice( trip_a, idx_out_a, num_a, idx_in_a, trip_b, idx_out_b, num_b, idx_in_b );

  return self;
}

/* @overload cross_splice_score( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__cross_splice_score( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_idx_in_a, VALUE rv_trip_b, VALUE rv_idx_out_b, VALUE rv_num_b, VALUE rv_idx_in_b ) {
  Trip *trip_a = get_trip_struct( self );

  assert_value_wraps_trip( rv_trip_b );
  Trip *trip_b = get_trip_struct( rv_trip_b );

  Trip ** both_trips = ALLOC_N( Trip*, 2 );

  both_trips[0] = trip_a;
  both_trips[1] = trip_b;

  gdcache__new_cache_from_trips( 2, both_trips );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_a = NUM2INT( rv_idx_in_a );
  int idx_out_b = NUM2INT( rv_idx_out_b );
  int num_b = NUM2INT( rv_num_b );
  int idx_in_b = NUM2INT( rv_idx_in_b );

  double score = trip__mutual_splice_score( trip_a, idx_out_a, num_a, idx_in_a, trip_b, idx_out_b, num_b, idx_in_b );

  xfree( both_trips );

  return DBL2NUM( score );
}

/* @overload splice_from_a_to_b( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__splice_from_a_to_b( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_trip_b, VALUE rv_idx_in_b ) {
  Trip *trip_a = get_trip_struct( self );

  assert_value_wraps_trip( rv_trip_b );
  Trip *trip_b = get_trip_struct( rv_trip_b );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_b = NUM2INT( rv_idx_in_b );

  trip__splice_from_a_to_b( trip_a, idx_out_a, num_a, trip_b, idx_in_b );

  return self;
}

/* @overload splice_from_a_to_b_score( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__splice_from_a_to_b_score( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_trip_b, VALUE rv_idx_in_b ) {
  Trip *trip_a = get_trip_struct( self );

  assert_value_wraps_trip( rv_trip_b );
  Trip *trip_b = get_trip_struct( rv_trip_b );

  Trip ** both_trips = ALLOC_N( Trip*, 2 );

  both_trips[0] = trip_a;
  both_trips[1] = trip_b;

  gdcache__new_cache_from_trips( 2, both_trips );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_b = NUM2INT( rv_idx_in_b );

  double score = trip__splice_from_a_to_b_score( trip_a, idx_out_a, num_a, trip_b, idx_in_b );

  xfree( both_trips );

  return DBL2NUM( score );

  return self;
}

/* @overload splice_self( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__splice_self( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_idx_in_a ) {
  Trip *trip_a = get_trip_struct( self );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_a = NUM2INT( rv_idx_in_a );

  trip__splice_self( trip_a, idx_out_a, num_a, idx_in_a );

  return self;
}

/* @overload splice_self_score( start_idx, num_splice_out, input_trip, output_trip )
 * Splices trip into this one, and returns an output of removed gifts
 * @return [StolenSleigh::Trip] removed gifts as a "remainder" trip
 */
VALUE trip_rbobject__splice_self_score( VALUE self, VALUE rv_idx_out_a, VALUE rv_num_a, VALUE rv_idx_in_b ) {
  Trip *trip_a = get_trip_struct( self );

  gdcache__new_cache_from_trips( 1, &trip_a );

  int idx_out_a = NUM2INT( rv_idx_out_a );
  int num_a = NUM2INT( rv_num_a );
  int idx_in_b = NUM2INT( rv_idx_in_b );

  double score = trip__splice_self_score( trip_a, idx_out_a, num_a, idx_in_b );

  return DBL2NUM( score );

  return self;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

void init_trip_class( ) {
  // Trip instantiation and class methods
  rb_define_alloc_func( StolenSleigh_Trip, trip_alloc );
  rb_define_method( StolenSleigh_Trip, "initialize", trip_rbobject__initialize, 1 );
  rb_define_method( StolenSleigh_Trip, "initialize_copy", trip_rbobject__initialize_copy, 1 );
  rb_define_singleton_method( StolenSleigh_Trip, "from_narray", trip_rbclass__from_narray, 1 );

  // Trip attributes
  rb_define_method( StolenSleigh_Trip, "num_gifts", trip_rbobject__get_num_gifts, 0 );
  rb_define_method( StolenSleigh_Trip, "gifts", trip_rbobject__get_narr_gifts, 0 );

  // Trip methods
  rb_define_method( StolenSleigh_Trip, "score", trip_rbobject__score, 0 );
  rb_define_method( StolenSleigh_Trip, "gifts_weight", trip_rbobject__gifts_weight, 0 );
  rb_define_method( StolenSleigh_Trip, "total_tension", trip_rbobject__total_tension, 0 );

  rb_define_method( StolenSleigh_Trip, "set_gift", trip_rbobject__set_gift, 2 );
  rb_define_method( StolenSleigh_Trip, "add_gift", trip_rbobject__add_gift, 2 );

  rb_define_method( StolenSleigh_Trip, "get_gift", trip_rbobject__get_gift, 1 );
  rb_define_method( StolenSleigh_Trip, "efficiency", trip_rbobject__efficiency, 0 );
  rb_define_method( StolenSleigh_Trip, "distance", trip_rbobject__distance, 0 );
  rb_define_method( StolenSleigh_Trip, "lowest_cost_insert_at", trip_rbobject__lowest_cost_insert_at, 1 );
  rb_define_method( StolenSleigh_Trip, "lowest_cost_break_at", trip_rbobject__lowest_cost_break_at, 0 );

  rb_define_method( StolenSleigh_Trip, "remove_gifts_by_idx", trip_rbobject__remove_gifts_by_idx, 1 );
  rb_define_method( StolenSleigh_Trip, "remove_gifts_by_gift_id", trip_rbobject__remove_gifts_by_gift_id, 1 );

  rb_define_method( StolenSleigh_Trip, "sample_gifts", trip_rbobject__sample_gifts, 2 );
  rb_define_method( StolenSleigh_Trip, "sample_gifts_var", trip_rbobject__sample_gifts_var, 2 );

  rb_define_method( StolenSleigh_Trip, "splice", trip_rbobject__splice, 4 );
  rb_define_method( StolenSleigh_Trip, "cross_splice", trip_rbobject__cross_splice, 7 );
  rb_define_method( StolenSleigh_Trip, "cross_splice_score", trip_rbobject__cross_splice_score, 7 );

  rb_define_method( StolenSleigh_Trip, "splice_from_a_to_b", trip_rbobject__splice_from_a_to_b, 4 );
  rb_define_method( StolenSleigh_Trip, "splice_from_a_to_b_score", trip_rbobject__splice_from_a_to_b_score, 4 );

  rb_define_method( StolenSleigh_Trip, "splice_self", trip_rbobject__splice_self, 3 );
  rb_define_method( StolenSleigh_Trip, "splice_self_score", trip_rbobject__splice_self_score, 3 );
  return;
}
