require 'stolen_sleigh'

# Takes solution of 12420021484 and optimises it to value of 12411851979
# Time: Roughly 2 days

srand( 341389562385908246906788 )
StolenSleigh.srand_array( [71334313,1944234,89133,12313421,9372102,537344145,424739,116650,82432126,46431734,134315] )

opts = Hash[
   :num_trips => 40, :inner_reps => 1000000, :draw => 5, :outer_reps => 106,
   :w_hav => 0.025, :w_long => 0.85, :w_cyl => 0.15, :adjust_every => 10,
   :start_t => 10000, :end_t => 20, :gift_choice => :shuffle_trips
]

t0 = Time.now

StolenSleigh::Optimiser.shaker_sa( 'output/submission_twenty_12420021484.csv', 'twenty', opts )

puts Time.now - t0
