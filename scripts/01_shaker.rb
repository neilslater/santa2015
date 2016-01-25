require 'stolen_sleigh'

# Takes basic solution of 1 trip per gift and optimises it to value of 12420021484.
# Time: 35727 (roughly 10 hours)

srand( 3413895678325658 )
StolenSleigh.srand_array( [71334313,1944234,81331231,9372102,537344145,424739,116650,82432126,46431734,134315] )

opts = Hash[
   :num_trips => 23, :inner_reps => 700, :draw => 4, :outer_reps => 100000,
   :w_hav => 0.025, :w_long => 0.85, :w_cyl => 0.15, :adjust_every => 10,
   :start_t => 50000, :end_t => 50, :gift_choice => :shuffle
]

t0 = Time.now

StolenSleigh::Optimiser.shaker( 'output/submission_29121011015.csv', '01', opts )

puts Time.now - t0
