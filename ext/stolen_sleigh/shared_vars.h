// ext/stolen_sleigh/shared_vars.h

#ifndef SHARED_VARS_H
#define SHARED_VARS_H

#include "ruby.h"

#define NUM2FLT(x) ((float)NUM2DBL(x))
#define FLT2NUM(x) (rb_float_new((double)x))

// Force inclusion of hash declarations (only Ruby MRI includes by default)
#ifdef HAVE_RUBY_ST_H
#include "ruby/st.h"
#else
#include "st.h"
#endif

extern VALUE StolenSleigh;
extern VALUE StolenSleigh_Input;
extern VALUE StolenSleigh_Solution;
extern VALUE StolenSleigh_Trip;
extern VALUE StolenSleigh_TripCollection;

extern VALUE StolenSleigh_Optimiser;
extern VALUE StolenSleigh_Optimiser_Shaker;
extern VALUE StolenSleigh_Optimiser_Splicer;

extern double* gifts;

#define LAT_OF_GIFT(id) gifts[ id * 8 ]
#define LONG_OF_GIFT(id) gifts[ id * 8 + 1 ]
#define WEIGHT_OF_GIFT(id) gifts[ id * 8 + 2 ]
#define NP_DISTANCE_TO_GIFT(id) gifts[ id * 8 + 3 ]
#define SOLO_SCORE_OF_GIFT(id) gifts[ id * 8 + 4 ]
#define X_OF_GIFT(id) gifts[ id * 8 + 5 ]
#define Y_OF_GIFT(id) gifts[ id * 8 + 6 ]
#define Z_OF_GIFT(id) gifts[ id * 8 + 7 ]

#define MAX_MULTI_TRIPS 400
#define MAX_MULTI_DRAW 50

#endif
