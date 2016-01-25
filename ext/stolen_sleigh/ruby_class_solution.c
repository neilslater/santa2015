// ext/stolen_sleigh/ruby_class_solution.c

#include "ruby_class_solution.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Ruby bindings for StolenSleigh::Solution - the deeper implementation is in
//  struct_solution.c
//

inline VALUE solution_as_ruby_class( Solution *solution , VALUE klass ) {
  return Data_Wrap_Struct( klass, solution__gc_mark, solution__destroy, solution );
}

VALUE solution_alloc(VALUE klass) {
  return solution_as_ruby_class( solution__create(), klass );
}

Solution *get_solution_struct( VALUE obj ) {
  Solution *solution;
  Data_Get_Struct( obj, Solution, solution );
  return solution;
}

void assert_value_wraps_solution( VALUE obj ) {
  if ( TYPE(obj) != T_DATA ||
      RDATA(obj)->dfree != (RUBY_DATA_FUNC)solution__destroy) {
    rb_raise( rb_eTypeError, "Expected a Solution object, but got something else" );
  }
}

/* Document-class: StolenSleigh::Solution
 *
 * This class models a solution for the challenge.
 *
 * An instance of this class represents the assignment of gifts to trips in the order that they
 * should be visited.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  StolenSleigh::Solution method definitions
//

/* @overload initialize(  )
 * Creates a new, empty solution.
 * @return [StolenSleigh::Solution] new instance
 */
VALUE solution_rbobject__initialize( VALUE self ) {
  Solution *solution = get_solution_struct( self );

  solution__init( solution );

  return self;
}

/* @overload clone
 * When cloned, the returned Solution has deep copies of all data.
 * @return [StolenSleigh::Solution] copy
 */
VALUE solution_rbobject__initialize_copy( VALUE copy, VALUE orig ) {
  Solution *solution_copy;
  Solution *solution_orig;

  if (copy == orig) return copy;
  solution_orig = get_solution_struct( orig );
  solution_copy = get_solution_struct( copy );

  solution__deep_copy( solution_copy, solution_orig );

  return copy;
}

/* @!attribute [r] items
 * The array of gifts and trips. Note that gift_ids are zero-indexed.
 * @return [NArray<int>] an array with shape [2, 100000]
 */
VALUE solution_rbobject__get_narr_items( VALUE self ) {
  Solution *solution = get_solution_struct( self );
  return solution->narr_items;
}

/* @overload validate
 * Raises an exception if there is a problem with missing or repeated gifts, or if any trip exceeds
 * the weight limit.
 * @return [true]
 */
VALUE solution_rbobject__validate( VALUE self ) {
  Solution *solution = get_solution_struct( self );
  if (! gifts) {
    rb_raise( rb_eRuntimeError, "Missing gifts array" );
  }

  if ( ! solution__has_all_gift_ids( solution ) ) {
    rb_raise( rb_eRuntimeError, "Not all gifts are in the solution." );
  }

  if ( ! solution__all_trips_under_weight_limit( solution ) ) {
    rb_raise( rb_eRuntimeError, "Non-valid trip found." );
  }

  return Qtrue;
}

/* @overload score
 * Returns current score of the solution. Returns 0.0 if solution is not valid.
 * @return [Float]
 */
VALUE solution_rbobject__score( VALUE self ) {
  Solution *solution = get_solution_struct( self );
  if (! gifts) {
    rb_raise( rb_eRuntimeError, "Missing gifts array" );
  }
  return DBL2NUM( solution__score( solution ) );
}

/* @overload from_narray
 * Creates new Solution object from supplied NArray. Used for serialisation.
 * @return [StolenSleigh::Solution] new instance
 */
VALUE solution_rbclass__from_narray( VALUE self, VALUE rv_narray ) {
  Solution *solution;
  struct NARRAY *na_items;
  volatile VALUE new_narray = na_cast_object(rv_narray, NA_LINT);
  GetNArray( new_narray, na_items );
  if ( na_items->rank != 2 ) {
    rb_raise( rb_eArgError, "Input solution array has wrong rank %d", na_items->rank );
  }
  if ( na_items->shape[0] != 2 || na_items->shape[1] != 100000 ) {
    rb_raise( rb_eArgError, "Input solution array is wrong shape (%d, %d)", na_items->shape[0], na_items->shape[1] );
  }
  // Maybe validate here, to prevent bad solution data propagating?

  VALUE rv_solution = solution_alloc( StolenSleigh_Solution );
  solution = get_solution_struct( rv_solution );

  solution__set_items( solution, rv_narray );

  return rv_solution;
}

/* @overload generate_trip( trip_id )
 * Returns a trip constructed from the data in the solution. This is a test method for
 * the internal mechanism for calculating the score.
 * @param [Integer] trip_id identifier for trip
 * @return [StolenSleigh::Trip]
 */
VALUE solution_rbobject__generate_trip( VALUE self, VALUE rv_trip_id ) {
  Solution *solution = get_solution_struct( self );
  Trip * trip = solution__generate_trip( solution, NUM2INT( rv_trip_id ), 0 );
  return Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy, trip );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void init_solution_class( ) {
  // Solution instantiation and class methods
  rb_define_alloc_func( StolenSleigh_Solution, solution_alloc );
  rb_define_method( StolenSleigh_Solution, "initialize", solution_rbobject__initialize, 0 );
  rb_define_method( StolenSleigh_Solution, "initialize_copy", solution_rbobject__initialize_copy, 1 );
  rb_define_singleton_method( StolenSleigh_Solution, "from_narray", solution_rbclass__from_narray, 1 );

  // Solution attributes
  rb_define_method( StolenSleigh_Solution, "items", solution_rbobject__get_narr_items, 0 );

  // Solution methods
  rb_define_method( StolenSleigh_Solution, "validate", solution_rbobject__validate, 0 );
  rb_define_method( StolenSleigh_Solution, "score", solution_rbobject__score, 0 );
  rb_define_method( StolenSleigh_Solution, "generate_trip", solution_rbobject__generate_trip, 1 );
}
