require 'stolen_sleigh'

opts = Hash[
  :num_trips => 120, :inner_reps => 1000000, :draw => 7, :outer_reps => 100,
  :w_hav => 0.025, :w_long => 0.85, :w_cyl => 0.15, :gift_choice => :tension,
  :start_t => 5000, :end_t => 50,
]

s = StolenSleigh::Solution.new

s.import_csv('output/submission_twenty_12411851979.csv')

tc = StolenSleigh::TripCollection.from_solution( s )

predicted_score = tc.score

checkpoint_score = StolenSleigh::Solver.next_checkpoint( predicted_score )

gift_tensions = tc.gift_tensions

trip_tensions = Hash[ (0..tc.max_trip_id).map { |id| if trip = tc.get_trip(id); [id,trip.total_tension]; else nil; end }.compact ]

trip_tensions.sort_by { |k,v| v }.reverse.each do |trip_id, tension|

  puts "- - - - - #{trip_id}, tension: #{tension}"

  trip_a_id = trip_id
  trip_a = tc.get_trip( trip_a_id )

  (0..11).each do |sample_idx|

    gift_id = trip_a.gifts[ ( (trip_a.num_gifts * sample_idx) / 12.0 ).to_i  ]

    trip_ids_to_opt = tc.pick_trip_group_closest_n_weighted( gift_id, opts[:num_trips], opts[:w_hav], opts[:w_long], opts[:w_cyl] )

    trips_to_opt = trip_ids_to_opt.map { |trip_id| tc.get_trip( trip_id ) }

    (1..119).each do |try_dist|

      trip_b = trips_to_opt[try_dist]
      trip_b_id = trip_ids_to_opt[try_dist]

      max_a = trip_a.num_gifts

      max_b = trip_b.num_gifts

      best_score = trip_a.score + trip_b.score
      orig_score = best_score

      i = 0
      best_idx_out_a = 0
      best_idx_out_b = 0
      best_num_a = 0
      best_num_b = 0

      (2...max_a-2).each do |idx_out_a|
        brmin = [idx_out_a-25,2].max
        brmax = [idx_out_a+30,max_b-2].min
        (brmin...brmax).each do |idx_out_b|
          num_a = max_a - idx_out_a
          num_b = max_b - idx_out_b
          i += 1
          puts i if (i % 1000000) == 0
          trip_a_tmp = trip_a.clone
          trip_b_tmp = trip_b.clone

          trip_a_tmp.cross_splice( idx_out_a, num_a, idx_out_a, trip_b_tmp, idx_out_b, num_b, idx_out_b )

          score = trip_a_tmp.score + trip_b_tmp.score

          if score < best_score
            best_score = score
            puts "#{trip_a_id}/#{trip_b_id} score #{best_score} - [#{idx_out_a},#{num_a}] & [#{idx_out_b},#{num_b}] - improvement #{orig_score - best_score}"
            best_idx_out_a = idx_out_a
            best_idx_out_b = idx_out_b
            best_num_a = num_a
            best_num_b = num_b
          end
        end
      end

      if ( (orig_score - best_score).abs > 0.01 )
        trip_a.cross_splice( best_idx_out_a, best_num_a, best_idx_out_a, trip_b, best_idx_out_b, best_num_b, best_idx_out_b )
        tc.set_trip( trip_a_id, trip_a )
        tc.set_trip( trip_b_id, trip_b )
        puts " saving improvement of #{(orig_score - best_score)}"
        puts tc.score

        if tc.score < checkpoint_score
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_twenty_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          checkpoint_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      end

    end

  end

end

new_solution = tc.to_solution
new_solution.validate
this_score = new_solution.score
submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_twenty_#{this_score.to_i}.csv" )
puts "Writing submission file: #{submission_file}"
new_solution.write_csv( submission_file )
