// ext/stolen_sleigh/ruby_class_input.c

#include "ruby_class_input.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Ruby bindings for functions of the input data - the deeper implementation is in
//  struct_input.c
//

inline VALUE input_as_ruby_class( Input *input , VALUE klass ) {
  return Data_Wrap_Struct( klass, input__gc_mark, input__destroy, input );
}

VALUE input_alloc(VALUE klass) {
  return input_as_ruby_class( input__create(), klass );
}

Input *get_input_struct( VALUE obj ) {
  Input *input;
  Data_Get_Struct( obj, Input, input );
  return input;
}

void assert_value_wraps_input( VALUE obj ) {
  if ( TYPE(obj) != T_DATA ||
      RDATA(obj)->dfree != (RUBY_DATA_FUNC)input__destroy) {
    rb_raise( rb_eTypeError, "Expected a Input object, but got something else" );
  }
}

/* Document-class: StolenSleigh::Input
 *
 * This class models the gifts data over which the cost should be minimised.
 *
 * An instance of this class represents the problem data for all 100,000 gifts. It includes a table
 * of gifts by id descibing gift location, weight and some pre-calculated values that get used
 * by solver code. Gift ids are zero-indexed in the array, so must be offset by 1 when importing
 * or exporting.
 *
 * For some methods, the instance will track gifts that have already been used, and return results
 * only including remaining gifts. This feature was used by some initial greedy algorithms, but is
 * no longer part of the solver.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  StolenSleigh::Input method definitions
//

/* @overload initialize(  )
 * Creates a new empty input object. When instantiated this way, all gift data is set to 0.0, and must be
 * filled in by reading or generating source data. The total number of gifts is hard-coded to 100,000.
 * @return [StolenSleigh::Input] new instance
 */
VALUE input_rbobject__initialize( VALUE self ) {
  Input *input = get_input_struct( self );

  input__init( input );
  input__init_used( input );

  return self;
}

/* @overload clone
 * When cloned, the returned StolenSleigh::Input has deep copies of all data.
 * @return [StolenSleigh::Input] copy of instance
 */
VALUE input_rbobject__initialize_copy( VALUE copy, VALUE orig ) {
  Input *input_copy;
  Input *input_orig;

  if (copy == orig) return copy;
  input_orig = get_input_struct( orig );
  input_copy = get_input_struct( copy );

  input__deep_copy( input_copy, input_orig );

  return copy;
}

/* @!attribute [r] gifts
 * The gift data over which the solution is optimised. This is a two-dimensional NArray with shape [8,100000].
 * input.gifts[0..7,0] corresponds to data about gift 1.
 *
 * The row data is:
 *
 *   **0** latitude in radians
 *   **1** longitude in radians
 *   **2** weight
 *   **3** distance from gift to North Pole (co-ordinates [PI,0])
 *   **4** cost of gift as a single trip
 *   **5** cartesian x co-ordinate on unit sphere (used to speed up haversine distance calculations)
 *   **6** cartesian y co-ordinate on unit sphere
 *   **7** cartesian z co-ordinate on unit sphere
 *
 * @return [NArray<dfloat>]
 */
VALUE input_rbobject__get_narr_gifts( VALUE self ) {
  Input *input = get_input_struct( self );
  return input->narr_gifts;
}

/* @overload from_narray
 * Creates new StolenSleigh::Input object directly from an NArray object of gifts. This can be used
 * to serialise input data.
 *
 * Note: Validation is minimal. You will get segfaults if the wrong kind or wrong size of NArray is used.
 *
 * @return [StolenSleigh::Input] new instance
 */
VALUE input_rbclass__from_narray( VALUE self, VALUE rv_narray ) {
  Input *input;
  struct NARRAY *na_gifts;
  VALUE rv_input = input_alloc( StolenSleigh_Input );
  input = get_input_struct( rv_input );

  input__set_gifts( input, rv_narray );
  input__init_used( input );

  return rv_input;
}

/* @overload sort_gifts_by_distance_from( gift_id, gift_ids_to_sort )
 * Sorts an array of gift_ids by their haversine distance from a "central" gift.
 *
 * @param [Integer] gift_id central gift
 * @param [NArray<int>] gift_ids_to_sort list of gift_ids that should be sorted according to distance
 * @return [NArray<int>] sorted array of gift_ids
 */
VALUE input_rbobject__sort_gifts_by_distance_from( VALUE self, VALUE rv_gift_id, VALUE rv_gift_ids ) {
  Input *input = get_input_struct( self );
  int * ids;
  struct NARRAY *na_ids;
  int num_gifts;

  volatile VALUE new_rv_ids = na_cast_object(rv_gift_ids, NA_LINT);
  GetNArray( new_rv_ids, na_ids );
  if ( na_ids->rank != 1 ) {
    rb_raise( rb_eArgError, "Array of ids has wrong rank %d", na_ids->rank );
  }
  num_gifts = na_ids->shape[0];
  ids = (int*) na_ids->ptr;

  input__sort_gifts_by_distance_from( input, NUM2INT(rv_gift_id), num_gifts, ids );

  return new_rv_ids;
}

/* @overload all_gifts_by_distance_from( gift_id )
 * Sorts all gift_ids by their haversine distance from a "central" gift.
 *
 * @param [Integer] gift_id central gift
 * @return [NArray<int>] sorted array of 99999 gift_ids
 */
VALUE input_rbobject__all_gifts_by_distance_from( VALUE self, VALUE rv_gift_id ) {
  Input *input = get_input_struct( self );
  int * ids, shape[1];
  struct NARRAY *na_ids;
  int num_gifts;
  char * exclusion_list = create_id_tracker();

  ids = input__remaining_gifts_by_distance_from( input, NUM2INT(rv_gift_id), exclusion_list, &num_gifts );

  shape[0] = num_gifts;
  volatile VALUE new_rv_ids = na_make_object( NA_LINT, 1, shape, cNArray );
  GetNArray( new_rv_ids, na_ids );
  memcpy( (int*) na_ids->ptr, ids, num_gifts * sizeof(int) );

  destroy_id_tracker( exclusion_list );
  xfree( ids );

  return new_rv_ids;
}


/* @overload all_gifts_by_long_only_distance_from( gift_id )
 * Sorts all gift_ids by their longitude-only distance from a "central" gift. This distance is measured
 * between each gift and its closest point on the line of longitude of the central gift.
 *
 * @param [Integer] gift_id central gift
 * @return [NArray<int>] sorted array of 99999 gift_ids
 */
VALUE input_rbobject__all_gifts_by_long_only_distance_from( VALUE self, VALUE rv_gift_id ) {
  Input *input = get_input_struct( self );
  int * ids, shape[1];
  struct NARRAY *na_ids;
  int num_gifts;
  char * exclusion_list = create_id_tracker();

  ids = input__remaining_gifts_by_long_only_distance_from( input, NUM2INT(rv_gift_id), exclusion_list, &num_gifts );

  shape[0] = num_gifts;
  volatile VALUE new_rv_ids = na_make_object( NA_LINT, 1, shape, cNArray );
  GetNArray( new_rv_ids, na_ids );
  memcpy( (int*) na_ids->ptr, ids, num_gifts * sizeof(int) );

  destroy_id_tracker( exclusion_list );
  xfree( ids );

  return new_rv_ids;
}

/* @overload all_gifts_by_weighted_distance_from( gift_id, w_hav, w_lng, w_cyl )
 * Sorts all gift_ids by a weighted distance metric from a "central" gift. This distance is measured
 * between each gift and its closest point on the line of longitude of the central gift.
 *
 * The "cylindrical distance" between gifts for the fourth parameter is the distance measured between
 * the gifts as if they kept the same longitude, but their latitude was set to 0.0, so it could also
 * be thought of as the "equator distance".
 *
 * @param [Integer] gift_id central gift
 * @param [Float] w_hav weighting for haversine distance
 * @param [Float] w_lng weighting for longitude-only distance
 * @param [Float] w_cyl weighting for "cylindrical distance"
 * @return [NArray<int>] sorted array of *remaining* gift_ids
 */
VALUE input_rbobject__all_gifts_by_weighted_distance_from( VALUE self, VALUE rv_gift_id, VALUE rv_w_hav, VALUE rv_w_lng, VALUE rv_w_cyl ) {
  Input *input = get_input_struct( self );
  int * ids, shape[1];
  struct NARRAY *na_ids;
  int num_gifts;
  char * exclusion_list = create_id_tracker();

  ids = input__remaining_gifts_by_weighted_distance_from( input, NUM2INT(rv_gift_id), exclusion_list, &num_gifts,
      NUM2DBL(rv_w_hav), NUM2DBL(rv_w_lng), NUM2DBL(rv_w_cyl) );

  shape[0] = num_gifts;
  volatile VALUE new_rv_ids = na_make_object( NA_LINT, 1, shape, cNArray );
  GetNArray( new_rv_ids, na_ids );
  memcpy( (int*) na_ids->ptr, ids, num_gifts * sizeof(int) );

  destroy_id_tracker( exclusion_list );
  xfree( ids );

  return new_rv_ids;
}

/* @overload all_gifts_within_bounds( lat_min, lat_max, lng_centre, lng_radius )
 * Finds all *remaining* gifts from the input that have co-ordinates within the supplied params.
 * @param [Float] lat_min minimum latitude
 * @param [Float] lat_max maximum latitude
 * @param [Float] lng_centre central longitude
 * @param [Float] lng_radius maximum difference from central longitude (wraps around globe)
 * @return [NArray<int>] array of *remaining* gift_ids that meet bounds constraints
 */
VALUE input_rbobject__all_gifts_within_bounds( VALUE self, VALUE rv_lat_min, VALUE rv_lat_max, VALUE rv_lng_centre, VALUE rv_lng_radius) {
  Input *input = get_input_struct( self );
  int * ids, shape[1];
  struct NARRAY *na_ids;
  int num_gifts;
  double lat_min, lat_max, lng_centre, lng_radius;
  lat_min = NUM2DBL( rv_lat_min );
  lat_max = NUM2DBL( rv_lat_max );
  lng_centre = NUM2DBL( rv_lng_centre );
  lng_radius = NUM2DBL( rv_lng_radius );

  char * exclusion_list = create_id_tracker();

  ids = input__remaining_gifts_within_bounds( input, lat_min, lat_max, lng_centre, lng_radius, exclusion_list, &num_gifts );

  shape[0] = num_gifts;
  volatile VALUE new_rv_ids = na_make_object( NA_LINT, 1, shape, cNArray );
  GetNArray( new_rv_ids, na_ids );
  memcpy( (int*) na_ids->ptr, ids, num_gifts * sizeof(int) );

  destroy_id_tracker( exclusion_list );
  xfree( ids );

  return new_rv_ids;
}


/* @overload most_costly_gift( )
 * Finds *remaining* gift that has highest cost if considered in its own trip.
 * @return [Integer] gift_id
 */
VALUE input_rbobject__most_costly_gift( VALUE self ) {
  Input *input = get_input_struct( self );
  int gift_id = input__remaining_most_costly_gift( input );

  if ( gift_id < 0 ) {
    return Qnil;
  }

  return INT2NUM( gift_id );
}

/* @overload furthest_gift( )
 * Finds *remaining* gift that is furthest away from the North Pole.
 * @return [Integer] gift_id
 */
VALUE input_rbobject__furthest_gift( VALUE self ) {
  Input *input = get_input_struct( self );
  int gift_id = input__remaining_furthest_gift( input );

  if ( gift_id < 0 ) {
    return Qnil;
  }

  return INT2NUM( gift_id );
}

/* @overload heaviest_gift( )
 * Finds *remaining* gift that has largest weight.
 * @return [Integer] gift_id
 */
VALUE input_rbobject__heaviest_gift( VALUE self ) {
  Input *input = get_input_struct( self );
  int gift_id = input__remaining_heaviest_gift( input );

  if ( gift_id < 0 ) {
    return Qnil;
  }

  return INT2NUM( gift_id );
}

/* @overload mark_as_used(  )
 * Marks supplied list of gift_ids as used, so they are not included in methods that work with "remaining"
 * gifts.
 * @param [NArray<int>] used_ids gift_ids that should be removed from consideration
 * @return [StolenSleigh::Input] self
 */
VALUE input_rbobject__mark_as_used( VALUE self, VALUE rv_used_ids ) {
  Input *input = get_input_struct( self );

  // TODO: Validate array
  struct NARRAY *narr;
  GetNArray( rv_used_ids, narr );
  input__mark_as_used( input, narr->shape[0], (int*) narr->ptr );

  return self;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void init_input_class( ) {
  // Input instantiation and class methods
  rb_define_alloc_func( StolenSleigh_Input, input_alloc );
  rb_define_method( StolenSleigh_Input, "initialize", input_rbobject__initialize, 0 );
  rb_define_method( StolenSleigh_Input, "initialize_copy", input_rbobject__initialize_copy, 1 );
  rb_define_singleton_method( StolenSleigh_Input, "from_narray", input_rbclass__from_narray, 1 );

  // Input attributes
  rb_define_method( StolenSleigh_Input, "gifts", input_rbobject__get_narr_gifts, 0 );

  // Input methods
  rb_define_method( StolenSleigh_Input, "sort_gifts_by_distance_from", input_rbobject__sort_gifts_by_distance_from, 2 );
  rb_define_method( StolenSleigh_Input, "all_gifts_by_distance_from", input_rbobject__all_gifts_by_distance_from, 1 );
  rb_define_method( StolenSleigh_Input, "all_gifts_by_long_only_distance_from", input_rbobject__all_gifts_by_long_only_distance_from, 1 );
  rb_define_method( StolenSleigh_Input, "all_gifts_by_weighted_distance_from", input_rbobject__all_gifts_by_weighted_distance_from, 4 );
  rb_define_method( StolenSleigh_Input, "all_gifts_within_bounds", input_rbobject__all_gifts_within_bounds, 4 );
  rb_define_method( StolenSleigh_Input, "most_costly_gift", input_rbobject__most_costly_gift, 0 );
  rb_define_method( StolenSleigh_Input, "furthest_gift", input_rbobject__furthest_gift, 0 );
  rb_define_method( StolenSleigh_Input, "heaviest_gift", input_rbobject__heaviest_gift, 0 );
  rb_define_method( StolenSleigh_Input, "mark_as_used", input_rbobject__mark_as_used, 1 );
}
