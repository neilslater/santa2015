require 'stolen_sleigh'

# Takes solution of 12410203718 and optimises it to value of ????
# Time: Roughly 2 days

srand( 956238590834138246906788 )
StolenSleigh.srand_array( [71334313,1944234,537344145,424739,116650,82432126,46431734,134315,89133,12313421,9372102] )

opts = Hash[
   :num_trips => 45, :inner_reps => 2000000, :draw => 3, :outer_reps => 200,
   :w_hav => 0.025, :w_long => 0.85, :w_cyl => 0.15, :adjust_every => 10,
   :start_t => 10000, :end_t => 20, :gift_choice => :shuffle_trips
]

t0 = Time.now

StolenSleigh::Optimiser.shaker_sa( 'output/submission_twenty_12410203718.csv', 'twenty', opts )

puts Time.now - t0
