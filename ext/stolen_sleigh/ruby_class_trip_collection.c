// ext/stolen_sleigh/ruby_class_trip_collection.c

#include "ruby_class_trip_collection.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Ruby bindings for StolenSleigh::TripCollection - the deeper implementation is in
//  struct_trip_collection.c
//

inline VALUE trip_collection_as_ruby_class( TripCollection *trip_collection , VALUE klass ) {
  return Data_Wrap_Struct( klass, trip_collection__gc_mark, trip_collection__destroy, trip_collection );
}

VALUE trip_collection_alloc(VALUE klass) {
  return trip_collection_as_ruby_class( trip_collection__create(), klass );
}

inline TripCollection *get_trip_collection_struct( VALUE obj ) {
  TripCollection *trip_collection;
  Data_Get_Struct( obj, TripCollection, trip_collection );
  return trip_collection;
}

void assert_value_wraps_trip_collection( VALUE obj ) {
  if ( TYPE(obj) != T_DATA ||
      RDATA(obj)->dfree != (RUBY_DATA_FUNC)trip_collection__destroy) {
    rb_raise( rb_eTypeError, "Expected a TripCollection object, but got something else" );
  }
}

/* Document-class: StolenSleigh::TripCollection
 * This class models a solution-in-progress, supporting changes to the contents of trips.
 *
 * An instance of the class represents positions of gifts within multiple trips. An instance can be constructed
 * from a StolenSleigh::Solution instance, or vice-versa.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Network method definitions
//

/* @overload initialize(  )
 * Creates a new empty collection of trips.
 * @return [StolenSleigh::TripCollection] new instance
 */
VALUE trip_collection_rbobject__initialize( VALUE self ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );

  trip_collection__init( trip_collection );

  return self;
}

/* @overload clone
 * When cloned, the new TripCollection has deep copies of all data.
 * @return [StolenSleigh::TripCollection] copy
 */
VALUE trip_collection_rbobject__initialize_copy( VALUE copy, VALUE orig ) {
  TripCollection *trip_collection_copy;
  TripCollection *trip_collection_orig;

  if (copy == orig) return copy;
  trip_collection_orig = get_trip_collection_struct( orig );
  trip_collection_copy = get_trip_collection_struct( copy );

  trip_collection__deep_copy( trip_collection_copy, trip_collection_orig );

  return copy;
}

/* @!attribute [r] num_trips
 * The current number of trips
 * @return [Integer]
 */
VALUE trip_collection_rbobject__get_num_trips( VALUE self ) {
  return INT2NUM( trip_collection__num_trips( get_trip_collection_struct( self ) )  );
}

/* @!attribute [r] max_trip_id
 * The maximum trip_id currently being tracked. Adding or removing trips may change this number
 * @return [Integer]
 */
VALUE trip_collection_rbobject__get_max_trip_id( VALUE self ) {
  return INT2NUM( get_trip_collection_struct( self )->max_trip_id - 1 );
}

/* @overload get_trip( trip_id )
 * Returns a **copy** of the trip stored at supplied key. Manipulating the returned trip has no effect
 * on the TripCollection.
 * @param [Integer] trip_id value between 0 and 99999 inclusive
 * @return [StolenSleigh::Trip,nil] copy of Trip at key, or nil if no such trip
 */
VALUE trip_collection_rbobject__get_trip( VALUE self, VALUE rv_idx ) {
  int i;
  volatile VALUE rv_trip = Qnil;
  TripCollection *trip_collection = get_trip_collection_struct( self );

  i = NUM2INT( rv_idx );

  if ( trip_collection__get_trip( trip_collection, i ) ) {
    rv_trip = Data_Wrap_Struct( StolenSleigh_Trip, trip__gc_mark, trip__destroy,
                                trip__clone( trip_collection__get_trip( trip_collection, i ) ) );
  }

  return rv_trip;
}

/* @overload set_trip( trip_id, trip )
 * Stores a **copy** of the trip at supplied key. Manipulating the trip afterwards has no effect
 * on the TripCollection.
 * @param [Integer] trip_id key between 0 and 99999 inclusive
 * @param [StolenSleigh::Trip] trip value to store
 * @return [Integer] trip_id
 */
VALUE trip_collection_rbobject__set_trip( VALUE self, VALUE rv_idx, VALUE rv_trip ) {
  int i;
  TripCollection *trip_collection = get_trip_collection_struct( self );
  i = NUM2INT( rv_idx );
  assert_value_wraps_trip( rv_trip );
  Trip * new_trip = trip__clone( get_trip_struct( rv_trip ) );

  i = trip_collection__set_trip( trip_collection, i, new_trip );

  if ( i < 0 ) {
    trip__destroy( new_trip );
  }

  return INT2NUM( i );
}

/* @overload add_trip( trip )
 * Stores a **copy** of the trip at a new key. Manipulating the trip afterwards has no effect
 * on the TripCollection.
 * @param [StolenSleigh::Trip] trip value to store
 * @return [Integer] trip_id new key, between 0 and 99999
 */
VALUE trip_collection_rbobject__add_trip( VALUE self, VALUE rv_trip ) {
  int i;
  TripCollection *trip_collection = get_trip_collection_struct( self );
  assert_value_wraps_trip( rv_trip );
  Trip * new_trip = trip__clone( get_trip_struct( rv_trip ) );

  i = trip_collection__add_trip( trip_collection, new_trip );

  if ( i < 0 ) {
    trip__destroy( new_trip );
  }

  return INT2NUM( i );
}

/* @overload delete_trip( trip_id )
 * Deletes trip stored against specific key.
 * @param [Integer] trip_id key between 0 and 99999 inclusive
 * @return [Boolean] true if a Trip was deleted, false otherwise
 */
VALUE trip_collection_rbobject__delete_trip( VALUE self, VALUE rv_idx ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );

  if ( trip_collection__delete_trip( trip_collection, NUM2INT( rv_idx ) ) ) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

/* @overload score( )
 * Calculates score.
 * @return [Float] total score for all trips in the TripCollection
 */
VALUE trip_collection_rbobject__score( VALUE self ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );
  if (! gifts) {
    rb_raise( rb_eRuntimeError, "Gifts array not initialised" );
  }
  return DBL2NUM( trip_collection__score( trip_collection ) );
}

/* @overload to_solution( )
 * Creates a solution from the trip data in the collection.
 * @return [StolenSleigh::Solution] new instance of Solution
 */
VALUE trip_collection_rbobject__to_solution( VALUE self ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );
  Solution * solution = solution__create();
  solution__init( solution );

  trip_collection__copy_to_solution( trip_collection, solution );

  return Data_Wrap_Struct( StolenSleigh_Solution, solution__gc_mark, solution__destroy, solution );
}

/* @overload from_solution( solution )
 * Creates trip collection structures from a solution object. Solution objects map directly to submission
 * CSV files, so this mechanism can be used to load a submission file as a StolenSleigh::Solution and
 * convert it into a structure that can be further optimised.
 * @param [StolenSleigh::Solution] solution
 * @return [StolenSleigh::TripCollection] new instance
 */
VALUE trip_collection_rbclass__from_solution( VALUE self, VALUE rv_solution ) {
  volatile VALUE rv_trip_collection;
  TripCollection *trip_collection;
  Solution * solution;

  assert_value_wraps_solution( rv_solution );
  solution = get_solution_struct( rv_solution );

  rv_trip_collection = trip_collection_alloc( StolenSleigh_TripCollection );
  trip_collection = get_trip_collection_struct( rv_trip_collection );

  trip_collection__init_from_solution( trip_collection, solution );

  return rv_trip_collection;
}

/* @overload trip_id_of_gift( gift_id )
 * Finds which trip a gift is currently assigned to.
 * @param [Integer] gift_id
 * @return [Integer,nil] trip_id of gift, or nil if it cannot be found
 */
VALUE trip_collection_rbobject__trip_id_of_gift( VALUE self, VALUE rv_gift_id ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );
  int trip_id = trip_collection__trip_id_of_gift( trip_collection, NUM2INT( rv_gift_id ) );

  if ( trip_id < 0 ) {
    return Qnil;
  }

  return INT2NUM( trip_id );
}

/* @overload insert_gift_at_lowest_cost( gift_id )
 * Adds gift to collection at the most optimal place in all the contained trips. This will not create
 * a new trip, so it is possible to fail due to no weight allowance left.
 *
 * @param [Integer] gift_id
 * @return [Float,nil] cost of inserting new gift, or nil if not possible to add gift
 */
VALUE trip_collection_rbobject__insert_gift_at_lowest_cost( VALUE self, VALUE rv_gift_id ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );
  int gift_id = NUM2INT( rv_gift_id );
  if ( gift_id < 0 || gift_id > 99999 ) {
    return Qnil;
  }

  double cost = trip_collection__insert_gift_at_lowest_cost( trip_collection, gift_id );

  if ( cost <= 0.0 ) {
    return Qnil;
  }

  return DBL2NUM( cost );
}

/* @overload gift_tensions()
 * A gift's "tension" is defined as the additional cost to deliver it due to indirect route in the
 * *forward part of the journey*. A high value implies that the gift might be better placed differently.
 *
 * The returned array is populated for all 100000 gifts, indexed by gift_id.
 *
 * @return [NArray<dfloat>] array of gift "tension", shape [100000]
 */
VALUE trip_collection_rbobject__gift_tensions( VALUE self ) {
  TripCollection *trip_collection = get_trip_collection_struct( self );
  int i,size = 100000;
  struct NARRAY *narr;
  double *tensions_ptr;
  volatile VALUE rv_result = na_make_object( NA_DFLOAT, 1, &size, cNArray );
  GetNArray( rv_result, narr );
  tensions_ptr = (double*) narr->ptr;
  for ( i = 0; i < 100000; i++ ) {
    tensions_ptr[i] = 0.0;
  }

  trip_collection__calc_gift_tensions( trip_collection, tensions_ptr );

  return rv_result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void init_trip_collection_class( ) {
  // TripCollection instantiation and class methods
  rb_define_alloc_func( StolenSleigh_TripCollection, trip_collection_alloc );
  rb_define_method( StolenSleigh_TripCollection, "initialize", trip_collection_rbobject__initialize, 0 );
  rb_define_method( StolenSleigh_TripCollection, "initialize_copy", trip_collection_rbobject__initialize_copy, 1 );
  rb_define_singleton_method( StolenSleigh_TripCollection, "from_solution", trip_collection_rbclass__from_solution, 1 );

  // TripCollection attributes
  rb_define_method( StolenSleigh_TripCollection, "num_trips", trip_collection_rbobject__get_num_trips, 0 );
  rb_define_method( StolenSleigh_TripCollection, "max_trip_id", trip_collection_rbobject__get_max_trip_id, 0 );

  // TripCollection methods
  rb_define_method( StolenSleigh_TripCollection, "get_trip", trip_collection_rbobject__get_trip, 1 );
  rb_define_method( StolenSleigh_TripCollection, "set_trip", trip_collection_rbobject__set_trip, 2 );
  rb_define_method( StolenSleigh_TripCollection, "add_trip", trip_collection_rbobject__add_trip, 1 );
  rb_define_method( StolenSleigh_TripCollection, "delete_trip", trip_collection_rbobject__delete_trip, 1 );
  rb_define_method( StolenSleigh_TripCollection, "score", trip_collection_rbobject__score, 0 );
  rb_define_method( StolenSleigh_TripCollection, "to_solution", trip_collection_rbobject__to_solution, 0 );
  rb_define_method( StolenSleigh_TripCollection, "trip_id_of_gift", trip_collection_rbobject__trip_id_of_gift, 1 );
  rb_define_method( StolenSleigh_TripCollection, "insert_gift_at_lowest_cost", trip_collection_rbobject__insert_gift_at_lowest_cost, 1 );
  rb_define_method( StolenSleigh_TripCollection, "gift_tensions", trip_collection_rbobject__gift_tensions, 0 );
}
