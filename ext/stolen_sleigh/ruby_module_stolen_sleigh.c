// ext/stolen_sleigh/ruby_module_stolen_sleigh.c

#include "ruby_module_stolen_sleigh.h"

double * gifts;

/* @overload srand( seed )
 * Seed the random number generator used inside StolenSleigh.
 * @param [Integer] seed 32-bit seed number
 * @return [nil]
 */
static VALUE rbmodule__srand( VALUE self, VALUE rv_seed ) {
  init_genrand( NUM2ULONG( rv_seed ) );
  return Qnil;
}

static unsigned long stolen_sleigh_srand_seed[640];

/* @overload srand_array( seed )
 * Seed the random number generator used inside StolenSleigh.
 * @param [Array<Integer>] seed an array of up to 640 times 32 bit seed numbers
 * @return [nil]
 */
static VALUE rbmodule__srand_array( VALUE self, VALUE rv_seed_array ) {
  int i, n;
  Check_Type( rv_seed_array, T_ARRAY );
  n = FIX2INT( rb_funcall( rv_seed_array, rb_intern("count"), 0 ) );
  if ( n < 1 ) {
    rb_raise( rb_eArgError, "empty array cannot be used to seed RNG" );
  }
  if ( n > 640 ) { n = 640; }
  for ( i = 0; i < n; i++ ) {
    stolen_sleigh_srand_seed[i] = NUM2ULONG( rb_ary_entry( rv_seed_array, i ) );
  }
  init_by_array( stolen_sleigh_srand_seed, n );
  return Qnil;
}

/* @overload rand( )
 * @!visibility private
 * Use the random number generator (Ruby binding only used for tests)
 * @return [Float] random number in range 0.0..1.0
 */
static VALUE rbmodule__rand_float( VALUE self ) {
  return FLT2NUM( genrand_real1() );
}

/* @overload haversine_distance( lat_a, long_a, lat_b, long_b )
 * Calculates haversine distance between two points described by lattitude and longitude in radians.
 * @param [Float] lat_a lattitude of point A
 * @param [Float] long_a longitude of point A
 * @param [Float] lat_b lattitude of point B
 * @param [Float] long_b longitude of point B
 * @return [Float] distance between A and B in kilometers
 */
static VALUE rbmodule__haversine_distance( VALUE self, VALUE rv_lat_a, VALUE rv_long_a, VALUE rv_lat_b, VALUE rv_long_b ) {
  return DBL2NUM( haversine_distance( NUM2DBL( rv_lat_a ), NUM2DBL( rv_long_a ),  NUM2DBL( rv_lat_b ), NUM2DBL( rv_long_b ) ) );
}

/* @overload gift_distance( gift_a_id, gift_b_id )
 * Calculates haversine distance between two points described by lattitude and longitude in radians.
 * @param [Integer] gift_a_id
 * @param [Integer] gift_b_id
 * @return [Float] distance between A and B in kilometers
 */
static VALUE rbmodule__gift_distance( VALUE self, VALUE rv_ga_id, VALUE rv_gb_id ) {
  double distance = distance_between_gifts( NUM2INT(rv_ga_id), NUM2INT(rv_gb_id) );
  return DBL2NUM( distance );
}


/* @overload sort_ids_by_float( ids, properties )
 * Sorts an array of ids by a list of properties
 * @param [NArray<int>] ids array of ids to sort
 * @param [NArray<float>] properties array where properties[ id ] contains sorted value
 * @return [NArray<int>] ids array sorted
 */
static VALUE rbmodule__sort_ids_by_float( VALUE self, VALUE rv_ids, VALUE rv_properties ) {
  struct NARRAY *na_ids;
  struct NARRAY *na_properties;
  int ids_size, max_id, i;
  int *ids;
  double *properties;

  volatile VALUE new_rv_ids = na_cast_object(rv_ids, NA_LINT);
  GetNArray( new_rv_ids, na_ids );
  if ( na_ids->rank != 1 ) {
    rb_raise( rb_eArgError, "Array of ids has wrong rank %d", na_ids->rank );
  }
  ids_size = na_ids->shape[0];
  ids = (int*) na_ids->ptr;

  max_id = 0;
  for ( i = 0; i < ids_size; i++ ) {
    if ( ids[i] < 0 ) {
      rb_raise( rb_eArgError, "Bad id value %d", ids[i] );
    }
    if ( ids[i] > max_id ) {
      max_id = ids[i];
    }
  }

  volatile VALUE new_rv_properties = na_cast_object(rv_properties, NA_DFLOAT);
  GetNArray( new_rv_properties, na_properties );
  if ( na_properties->rank != 1 ) {
    rb_raise( rb_eArgError, "Array of properties has wrong rank %d", na_properties->rank );
  }
  if ( max_id >= na_properties->shape[0] ) {
    rb_raise( rb_eArgError, "Array of properties is length %d, but highest id to sort is %d", na_properties->shape[0], max_id );
  }
  properties = (double*) na_properties->ptr;

  quicksort_ids_by_double( ids, properties, 0, ids_size - 1 );

  return new_rv_ids;
}

/* @overload init_gifts( )
 * Loads current input data for access by C code
 * @return [true]
 */
static VALUE rbmodule__init_gifts( VALUE self ) {
  struct NARRAY *na_ids;
  volatile VALUE rv_input = rb_const_get( StolenSleigh, rb_intern("INPUT") );
  assert_value_wraps_input( rv_input );
  Input *input = get_input_struct( rv_input );
  gifts = input->gifts;
  return Qtrue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VALUE StolenSleigh = Qnil;
VALUE StolenSleigh_Input = Qnil;
VALUE StolenSleigh_Solution = Qnil;
VALUE StolenSleigh_Trip = Qnil;
VALUE StolenSleigh_TripCollection = Qnil;

VALUE StolenSleigh_Optimiser = Qnil;
VALUE StolenSleigh_Optimiser_Shaker = Qnil;
VALUE StolenSleigh_Optimiser_Splicer = Qnil;

void init_module_stolen_sleigh() {
  StolenSleigh = rb_define_module( "StolenSleigh" );
  StolenSleigh_Input = rb_define_class_under( StolenSleigh, "Input", rb_cObject );
  StolenSleigh_Solution = rb_define_class_under( StolenSleigh, "Solution", rb_cObject );
  StolenSleigh_Trip = rb_define_class_under( StolenSleigh, "Trip", rb_cObject );
  StolenSleigh_TripCollection = rb_define_class_under( StolenSleigh, "TripCollection", rb_cObject );

  StolenSleigh_Optimiser = rb_define_module_under( StolenSleigh, "Optimiser" );
  StolenSleigh_Optimiser_Shaker = rb_define_module_under( StolenSleigh_Optimiser, "Shaker" );
  StolenSleigh_Optimiser_Splicer = rb_define_module_under( StolenSleigh_Optimiser, "Splicer" );

  rb_define_singleton_method( StolenSleigh, "srand", rbmodule__srand, 1 );
  rb_define_singleton_method( StolenSleigh, "srand_array", rbmodule__srand_array, 1 );
  rb_define_singleton_method( StolenSleigh, "rand", rbmodule__rand_float, 0 );
  rb_define_singleton_method( StolenSleigh, "haversine_distance", rbmodule__haversine_distance, 4 );
  rb_define_singleton_method( StolenSleigh, "sort_ids_by_float", rbmodule__sort_ids_by_float, 2 );
  rb_define_singleton_method( StolenSleigh, "gift_distance", rbmodule__gift_distance, 2 );
  rb_define_singleton_method( StolenSleigh, "init_gifts", rbmodule__init_gifts, 0 );

  init_input_class();
  init_solution_class();
  init_trip_class();
  init_trip_collection_class();
  init_srand_by_time();
  init_module_optimiser(); // Also inits sub-modules
}
