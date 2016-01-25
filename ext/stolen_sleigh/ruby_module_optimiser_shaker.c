// ext/stolen_sleigh/ruby_module_optimiser_shaker.c

#include "ruby_module_optimiser_shaker.h"

/* @overload test_multi_draw( trips, max_draw )
 * Test routine checks ability of core code to make selection of gift_ids from complex structure.
 * This is limited to maximum 30 Trips to sample from, and 50 items drawn from each trip.
 * @param [Array<StolenSleigh::Trip>] trips original pool of trips to draw from
 * @param [Integer] max_draw maximum number of items to draw per trip
 * @return [Array<StolenSleigh::Trip>>, Array<int>] the remaining, and drawn ids
 */
static VALUE optimiser_shaker_rbmodule__test_multi_draw( VALUE self, VALUE rv_array_of_trips, VALUE rv_max_draw ) {
  int i, n, total_num_draws, max_draw, *tmp_ptr;
  volatile VALUE rv_trip;
  Trip ** input_trips;
  Trip ** remaining_trips;
  Trip * tmp_trip;
  struct NARRAY *na_gift_ids;

  max_draw = NUM2INT( rv_max_draw );
  if ( max_draw < 1 || max_draw > 50 ) {
    rb_raise( rb_eArgError, "max_draw value %d out of bounds 1..50", max_draw );
  }

  Check_Type( rv_array_of_trips, T_ARRAY );
  n = FIX2INT( rb_funcall( rv_array_of_trips, rb_intern("count"), 0 ) );
  if ( n < 1 ) {
    rb_raise( rb_eArgError, "empty array cannot be used in test_multi_draw" );
  }
  if ( n > 390 ) {
    rb_raise( rb_eArgError, "input array too large in test_multi_draw" );
  }

  // 1. Validate all items are Trips
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    assert_value_wraps_trip( rv_trip );
  }

  // 2. Load trip data
  input_trips = ALLOC_N( Trip*, n );
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    input_trips[i] = get_trip_struct( rv_trip );
  }

  // 3. Run the internal method:
  total_num_draws = optimiser_shaker__multi_trip_sample( n, input_trips, max_draw );

  // 4. Access remaining pool and convert to Array of Trips
  volatile VALUE rv_remaining_pool = rb_ary_new2( n );
  remaining_trips = optimiser_shaker__get_tmp_trips();
  for ( i = 0; i < n; i++ ) {
    tmp_trip = trip__clone( remaining_trips[i] );
    rb_ary_store( rv_remaining_pool, i, Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy, tmp_trip ) );
  }

  // 5. Access combined drawn pool and convert to NArray
  volatile VALUE rv_drawn_combined = na_make_object( NA_LINT, 1, &total_num_draws, cNArray );
  tmp_ptr = optimiser_shaker__get_gift_pool();
  GetNArray( rv_drawn_combined, na_gift_ids );
  memcpy( (int*) na_gift_ids->ptr, tmp_ptr, total_num_draws * sizeof(int) );

  // 6. Clean up
  xfree( input_trips );

  // 7. Return Array of data
  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, rv_remaining_pool );
  rb_ary_store( rv_result, 1, rv_drawn_combined );
  return rv_result;
}


/* @overload multi_trip_shake( array_of_trips, reps, max_draw )
 * Test routine checks ability of core code to make selection of gift_ids from complex structure.
 * This is limited to maximum 10 NArrays to sample from, and 10 items drawn from each array
 * @param [Array<StolenSleigh::Trip>] array_of_trips original pool
 * @param [Integer] reps number of
 * @param [Integer] max_draw maximum number of items to draw
 * @return [Array<StolenSleigh::Trip>, Float] the rearranged trips and their total score
 */
static VALUE optimiser_shaker_rbmodule__multi_trip_shake( VALUE self, VALUE rv_array_of_trips, VALUE rv_reps, VALUE rv_max_draw ) {
  int i, n, total_num_draws, reps, max_draw, *tmp_ptr;
  volatile VALUE rv_trip;
  Trip ** input_trips;
  Trip ** remaining_trips;
  Trip * tmp_trip;
  struct NARRAY *na_gift_ids;
  double score;

  max_draw = NUM2INT( rv_max_draw );
  if ( max_draw < 1 || max_draw > 50 ) {
    rb_raise( rb_eArgError, "max_draw value %d out of bounds 1..50", max_draw );
  }

  reps = NUM2INT( rv_reps );

  Check_Type( rv_array_of_trips, T_ARRAY );
  n = FIX2INT( rb_funcall( rv_array_of_trips, rb_intern("count"), 0 ) );
  if ( n < 1 ) {
    rb_raise( rb_eArgError, "empty array cannot be used in test_multi_draw" );
  }
  if ( n > 390 ) {
    rb_raise( rb_eArgError, "input array too large in test_multi_draw" );
  }

  // 1. Validate all items are Trips
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    assert_value_wraps_trip( rv_trip );
  }

  // 2. Load trip data
  input_trips = ALLOC_N( Trip*, n );
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    input_trips[i] = trip__clone( get_trip_struct( rv_trip ) );
  }

  // 3. Run the internal method:
  score = optimiser_shaker__multi_trip_shake( n, input_trips, max_draw, reps );

  // 4. Return the altered trips
  volatile VALUE rv_result_trips = rb_ary_new2( n );
  for ( i = 0; i < n; i++ ) {
    tmp_trip = input_trips[i];
    rb_ary_store( rv_result_trips, i, Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy, tmp_trip ) );
  }

  // 5. Clean up
  xfree( input_trips );

  // 6. Return Array of data
  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, rv_result_trips );
  rb_ary_store( rv_result, 1, DBL2NUM( score ) );
  return rv_result;
}


/* @overload multi_trip_shake_sa( array_of_trips, reps, max_draw, start_t, end_t )
 * Test routine checks ability of core code to make selection of gift_ids from complex structure.
 * This is limited to maximum 10 NArrays to sample from, and 10 items drawn from each array
 * @param [Array<StolenSleigh::Trip>] array_of_trips original pool
 * @param [Integer] reps number of
 * @param [Integer] max_draw maximum number of items to draw
 * @return [Array<StolenSleigh::Trip>, Float] the rearranged trips and their total score
 */
static VALUE optimiser_shaker_rbmodule__multi_trip_shake_sa( VALUE self, VALUE rv_array_of_trips, VALUE rv_reps, VALUE rv_max_draw, VALUE rv_start_t, VALUE rv_end_t ) {
  int i, n, total_num_draws, reps, max_draw, *tmp_ptr;
  volatile VALUE rv_trip;
  Trip ** input_trips;
  Trip ** remaining_trips;
  Trip * tmp_trip;
  struct NARRAY *na_gift_ids;
  double score, start_t, end_t, invt, t_factor;

  max_draw = NUM2INT( rv_max_draw );
  if ( max_draw < 1 || max_draw > 50 ) {
    rb_raise( rb_eArgError, "max_draw value %d out of bounds 1..50", max_draw );
  }

  reps = NUM2INT( rv_reps );
  start_t = NUM2DBL( rv_start_t );
  end_t = NUM2DBL( rv_end_t );

  // Easy-to-set temperature params converted to easy-to-use temp params
  invt = 1.0/start_t;
  t_factor = 1.0/( exp( log( end_t/start_t ) / reps ) );

  Check_Type( rv_array_of_trips, T_ARRAY );
  n = FIX2INT( rb_funcall( rv_array_of_trips, rb_intern("count"), 0 ) );
  if ( n < 1 ) {
    rb_raise( rb_eArgError, "empty array cannot be used in test_multi_draw" );
  }
  if ( n > 390 ) {
    rb_raise( rb_eArgError, "input array too large in test_multi_draw" );
  }

  // 1. Validate all items are Trips
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    assert_value_wraps_trip( rv_trip );
  }

  // 2. Load trip data
  input_trips = ALLOC_N( Trip*, n );
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    input_trips[i] = trip__clone( get_trip_struct( rv_trip ) );
  }

  // 3. Run the internal method:
  score = optimiser_shaker__multi_trip_shake_sa( n, input_trips, max_draw, reps, invt, t_factor );

  // 4. Return the altered trips
  volatile VALUE rv_result_trips = rb_ary_new2( n );
  for ( i = 0; i < n; i++ ) {
    tmp_trip = input_trips[i];
    rb_ary_store( rv_result_trips, i, Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy, tmp_trip ) );
  }

  // 5. Clean up
  xfree( input_trips );

  // 6. Return Array of data
  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, rv_result_trips );
  rb_ary_store( rv_result, 1, DBL2NUM( score ) );
  return rv_result;
}



/* @overload simple_shake_sa( array_of_trips, reps, max_draw, start_t, end_t )
 * Test routine checks ability of core code to make selection of gift_ids from complex structure.
 * This is limited to maximum 10 NArrays to sample from, and 10 items drawn from each array
 * @param [Array<StolenSleigh::Trip>] array_of_trips original pool
 * @param [Integer] reps number of
 * @param [Integer] max_draw maximum number of items to draw
 * @return [Array<StolenSleigh::Trip>, Float] the rearranged trips and their total score
 */
static VALUE optimiser_shaker_rbmodule__simple_shake_sa( VALUE self, VALUE rv_array_of_trips, VALUE rv_reps, VALUE rv_max_draw, VALUE rv_start_t, VALUE rv_end_t ) {
  int i, n, total_num_draws, reps, max_draw, *tmp_ptr;
  volatile VALUE rv_trip;
  Trip ** input_trips;
  Trip ** remaining_trips;
  Trip * tmp_trip;
  struct NARRAY *na_gift_ids;
  double score, start_t, end_t, invt, t_factor;

  max_draw = NUM2INT( rv_max_draw );
  if ( max_draw < 1 || max_draw > 50 ) {
    rb_raise( rb_eArgError, "max_draw value %d out of bounds 1..50", max_draw );
  }

  reps = NUM2INT( rv_reps );
  start_t = NUM2DBL( rv_start_t );
  end_t = NUM2DBL( rv_end_t );

  // Easy-to-set temperature params converted to easy-to-use temp params
  invt = 1.0/start_t;
  t_factor = 1.0/( exp( log( end_t/start_t ) / reps ) );

  Check_Type( rv_array_of_trips, T_ARRAY );
  n = FIX2INT( rb_funcall( rv_array_of_trips, rb_intern("count"), 0 ) );
  if ( n < 1 ) {
    rb_raise( rb_eArgError, "empty array cannot be used in test_multi_draw" );
  }
  if ( n > 390 ) {
    rb_raise( rb_eArgError, "input array too large in test_multi_draw" );
  }

  // 1. Validate all items are Trips
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    assert_value_wraps_trip( rv_trip );
  }

  // 2. Load trip data
  input_trips = ALLOC_N( Trip*, n );
  for ( i = 0; i < n; i++ ) {
    rv_trip = rb_ary_entry( rv_array_of_trips, i );
    input_trips[i] = trip__clone( get_trip_struct( rv_trip ) );
  }

  // 3. Run the internal method:
  score = optimiser_shaker__simple_shake_sa( n, input_trips, max_draw, reps, invt, t_factor );

  // 4. Return the altered trips
  volatile VALUE rv_result_trips = rb_ary_new2( n );
  for ( i = 0; i < n; i++ ) {
    tmp_trip = input_trips[i];
    rb_ary_store( rv_result_trips, i, Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy, tmp_trip ) );
  }

  // 5. Clean up
  xfree( input_trips );

  // 6. Return Array of data
  volatile VALUE rv_result = rb_ary_new2( 2 );
  rb_ary_store( rv_result, 0, rv_result_trips );
  rb_ary_store( rv_result, 1, DBL2NUM( score ) );
  return rv_result;
}


void init_module_optimiser_shaker() {
  rb_define_singleton_method( StolenSleigh_Optimiser_Shaker, "test_multi_draw", optimiser_shaker_rbmodule__test_multi_draw, 2 );
  rb_define_singleton_method( StolenSleigh_Optimiser_Shaker, "multi_trip_shake",  optimiser_shaker_rbmodule__multi_trip_shake, 3 );
  rb_define_singleton_method( StolenSleigh_Optimiser_Shaker, "multi_trip_shake_sa",  optimiser_shaker_rbmodule__multi_trip_shake_sa, 5 );
  rb_define_singleton_method( StolenSleigh_Optimiser_Shaker, "simple_shake_sa",  optimiser_shaker_rbmodule__simple_shake_sa, 5 );

  optimiser_shaker__init_buffers();
  return;
}
