// ext/stolen_sleigh/ruby_module_optimiser_splicer.c

#include "ruby_module_optimiser_splicer.h"

/* @overload multi_trip_splice_sa( array_of_trips, reps, max_draw, start_t, end_t )
 * Test routine checks ability of core code to make selection of gift_ids from complex structure.
 * This is limited to maximum 10 NArrays to sample from, and 10 items drawn from each array
 * @param [Array<StolenSleigh::Trip>] array_of_trips original pool
 * @param [Integer] reps number of
 * @param [Integer] max_draw maximum number of items to draw
 * @return [Array<StolenSleigh::Trip>, Float] the rearranged trips and their total score
 */
static VALUE optimiser_shaker_rbmodule__multi_trip_splice_sa( VALUE self, VALUE rv_array_of_trips, VALUE rv_reps, VALUE rv_max_draw, VALUE rv_start_t, VALUE rv_end_t ) {
  int i, n, total_num_draws, reps, max_draw, *tmp_ptr;
  volatile VALUE rv_trip;
  Trip ** input_trips;
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
  if ( n > 75 ) {
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
  score = optimiser_splicer__multi_trip_splice_sa(  n, input_trips, max_draw, reps, invt, t_factor );

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


void init_module_optimiser_splicer() {
  rb_define_singleton_method( StolenSleigh_Optimiser_Splicer, "multi_trip_splice_sa", optimiser_shaker_rbmodule__multi_trip_splice_sa, 5 );
  return;
}
