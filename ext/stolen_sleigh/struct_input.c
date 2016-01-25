// ext/stolen_sleigh/struct_input.c

#include "struct_input.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definitions for Input memory management
//

Input *input__create() {
  Input *input;
  input = xmalloc( sizeof(Input) );
  input->gifts_shape = NULL;
  input->narr_gifts = Qnil;
  input->gifts = NULL;
  input->used = NULL;
  return input;
}

void input__init( Input *input ) {
  int i;
  struct NARRAY *narr;
  double *narr_gifts_ptr;

  input->gifts_shape = ALLOC_N( int, 2 );
  input->gifts_shape[0] = 8;
  input->gifts_shape[1] = 100000;
  input->narr_gifts = na_make_object( NA_DFLOAT, 2, input->gifts_shape, cNArray );
  GetNArray( input->narr_gifts, narr );
  narr_gifts_ptr = (double*) narr->ptr;
  for( i = 0; i < narr->total; i++ ) {
    narr_gifts_ptr[i] = 0.0;
  }
  input->gifts = (double *) narr->ptr;

  return;
}

void input__init_used( Input *input ) {
  xfree( input->used );
  input->used = create_id_tracker();
  return;
}

void input__mark_as_used( Input *input, int num_gifts, int * used_gift_ids ) {
  if ( ! input->used ) {
    input->used = create_id_tracker();
  }
  int i;
  for ( i = 0; i < num_gifts; i++ ) {
    input->used[ used_gift_ids[i] ] = 1;
  }
  return;
}

void input__set_gifts( Input *input, VALUE rv_gifts ) {
  int i;
  struct NARRAY *narr;
  double *narr_gifts_ptr;

  input->gifts_shape = ALLOC_N( int, 2 );
  input->gifts_shape[0] = 8;
  input->gifts_shape[1] = 100000;
  input->narr_gifts = rv_gifts;
  GetNArray( input->narr_gifts, narr );
  input->gifts = (double *) narr->ptr;

  return;
}

void input__destroy( Input *input ) {
  xfree( input->used );
  xfree( input->gifts_shape );
  xfree( input );
  return;
}

void input__gc_mark( Input *input ) {
  rb_gc_mark( input->narr_gifts );
  return;
}

void input__deep_copy( Input *input_copy, Input *input_orig ) {
  struct NARRAY *narr;

  input_copy->narr_gifts = na_clone( input_orig->narr_gifts );
  GetNArray( input_copy->narr_gifts, narr );
  input_copy->gifts = (double *) narr->ptr;
  input_copy->gifts_shape = ALLOC_N( int, 2 );
  memcpy( input_copy->gifts_shape, input_orig->gifts_shape, 2 * sizeof(int) );
  input_copy->used = create_id_tracker();
  memcpy( input_copy->used, input_orig->used, 100000 * sizeof(char) );

  return;
}

Input * input__clone( Input *input_orig ) {
  Input * input_copy = input__create();
  input__deep_copy( input_copy, input_orig );
  return input_copy;
}

void input__sort_gifts_by_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids ) {
  int i, gift_id;
  double * distances = ALLOC_N( double, 100000 );
  for ( i = 0 ; i < num_gifts; i++ ) {
    gift_id = gift_ids[i];
    distances[ gift_id ] = cartesian_distance_metric( gift_id, central_gift_id );
  }

  quicksort_ids_by_double( gift_ids, distances, 0, num_gifts - 1 );

  xfree(distances);

  return;
}

void input__sort_gifts_by_long_only_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids ) {
  int i, gift_id;
  double central_gift_lng = LONG_OF_GIFT( central_gift_id );
  double * distances = ALLOC_N( double, 100000 );
  for ( i = 0 ; i < num_gifts; i++ ) {
    gift_id = gift_ids[i];
    distances[ gift_id ] = distance_between_gift_and_longitude( gift_id, central_gift_lng );
  }

  quicksort_ids_by_double( gift_ids, distances, 0, num_gifts - 1 );

  xfree(distances);

  return;
}


void input__sort_gifts_by_weighted_distance_from( Input *input, int central_gift_id, int num_gifts, int * gift_ids, double w_hav, double w_long_only, double w_cyl ) {
  int i, gift_id;
  double * distances = ALLOC_N( double, 100000 );
  for ( i = 0 ; i < num_gifts; i++ ) {
    gift_id = gift_ids[i];
    distances[ gift_id ] = weighted_metric_distance_between_gifts( gift_id, central_gift_id, w_hav, w_long_only, w_cyl );
  }

  quicksort_ids_by_double( gift_ids, distances, 0, num_gifts - 1 );

  xfree(distances);

  return;
}

int * input__remaining_gifts_by_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts ) {
  int i, n = 100000;
  int *gift_ids;
  for ( i = 0; i < 100000; i++ ) {
    n -= exclusion_list[i];
  }
  if ( ! exclusion_list[central_gift_id] ) {
    n -= 1;
  }

  *num_gifts = n;
  gift_ids = ALLOC_N( int, n );
  n = 0;
  for ( i = 0; i < 100000; i++ ) {
    if ( exclusion_list[i] || i == central_gift_id ) {
      continue;
    }
    gift_ids[n] = i;
    n++;
  }
  input__sort_gifts_by_distance_from( input, central_gift_id, n, gift_ids );
  return gift_ids;
}

int * input__remaining_gifts_by_long_only_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts ) {
  int i, n = 100000;
  int *gift_ids;
  for ( i = 0; i < 100000; i++ ) {
    n -= exclusion_list[i];
  }
  if ( ! exclusion_list[central_gift_id] ) {
    n -= 1;
  }

  *num_gifts = n;
  gift_ids = ALLOC_N( int, n );
  n = 0;
  for ( i = 0; i < 100000; i++ ) {
    if ( exclusion_list[i] || i == central_gift_id ) {
      continue;
    }
    gift_ids[n] = i;
    n++;
  }
  input__sort_gifts_by_long_only_distance_from( input, central_gift_id, n, gift_ids );
  return gift_ids;
}

int * input__remaining_gifts_by_weighted_distance_from( Input *input, int central_gift_id, char *exclusion_list, int *num_gifts, double w_hav, double w_long_only, double w_cyl ) {
  int i, n = 100000;
  int *gift_ids;
  for ( i = 0; i < 100000; i++ ) {
    n -= exclusion_list[i];
  }
  if ( ! exclusion_list[central_gift_id] ) {
    n -= 1;
  }

  *num_gifts = n;
  gift_ids = ALLOC_N( int, n );
  n = 0;
  for ( i = 0; i < 100000; i++ ) {
    if ( exclusion_list[i] || i == central_gift_id ) {
      continue;
    }
    gift_ids[n] = i;
    n++;
  }
  input__sort_gifts_by_weighted_distance_from( input, central_gift_id, n, gift_ids, w_hav, w_long_only, w_cyl );
  return gift_ids;
}

int * input__remaining_gifts_within_bounds( Input *input, double lat_min, double lat_max, double lng_centre, double lng_radius, char *exclusion_list, int *num_gifts ) {
  int i, gift_offset, n = 0;
  int *gift_ids;
  double gift_lat, gift_lng;
  double lng_min, lng_max;
  int wrap_hi = 0;
  int wrap_lo = 0;
  lng_max = lng_centre + lng_radius;
  if ( lng_max > PI ) {
    wrap_hi = 1;
  }
  lng_min = lng_centre - lng_radius;
  if ( lng_min < -PI ) {
    wrap_lo = 1;
  }

  gift_ids = ALLOC_N( int, 100000 );
  for ( i = 0; i < 100000; i++ ) {
    if ( exclusion_list[i] ) {
      continue;
    }
    gift_offset = i * 8;
    gift_lat = input->gifts[ gift_offset ];
    if ( gift_lat > lat_max || gift_lat < lat_min ) {
      continue;
    }

    gift_lng = input->gifts[ gift_offset + 1];
    if ( gift_lng > lng_max ) {
      if ( ! wrap_lo || gift_lng < (lng_min + TWO_PI) ) {
        continue;
      }
    }
    if ( gift_lng < lng_min ) {
      if ( ! wrap_hi || gift_lng > (lng_max - TWO_PI) ) {
        continue;
      }
    }

    gift_ids[n] = i;
    n++;
  }

  *num_gifts = n;

  return gift_ids;
}

int input__remaining_most_costly_gift( Input *input ) {
  int i, gift_id = -1;
  double cost, best_cost = 0.0;
  for ( i = 0; i < 100000; i++ ) {
    if ( input->used[i] ) {
      continue;
    }
    cost = input->gifts[ 8 * i + 4 ];
    if ( cost > best_cost ) {
      best_cost = cost;
      gift_id = i;
    }
  }

  return gift_id;
}

int input__remaining_heaviest_gift( Input *input ) {
  int i, gift_id = -1;
  double weight, best_weight = 0.0;
  for ( i = 0; i < 100000; i++ ) {
    if ( input->used[i] ) {
      continue;
    }
    weight = input->gifts[ 8 * i + 2 ];
    if ( weight > best_weight ) {
      best_weight = weight;
      gift_id = i;
    }
  }

  return gift_id;
}

int input__remaining_furthest_gift( Input *input ) {
  int i, gift_id = -1;
  double dist, best_dist = 0.0;
  for ( i = 0; i < 100000; i++ ) {
    if ( input->used[i] ) {
      continue;
    }
    dist = input->gifts[ 8 * i + 3 ];
    if ( dist > best_dist ) {
      best_dist = dist;
      gift_id = i;
    }
  }

  return gift_id;
}
