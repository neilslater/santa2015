require 'stolen_sleigh'

srand( 27289123183321245365044 )

s = StolenSleigh::Solution.new

s.import_csv('output/submission_twenty_12411516411.csv')

tc = StolenSleigh::TripCollection.from_solution( s )

score = tc.score

loop do
  loop_start_score = tc.score

  sequence = [*0..99999].shuffle

  sequence.each_with_index do |gift_id,i|
    # Find and remove existing gift
    trip_id = tc.trip_id_of_gift( gift_id )
    trip = tc.get_trip( trip_id )

    before_score = trip.score
    trip.remove_gifts_by_gift_id( NArray[gift_id] )
    delta = trip.score - before_score
    tc.set_trip( trip_id, trip )

    # Add it back
    delta += tc.insert_gift_at_lowest_cost( gift_id )

    if (delta > 1e-6)
      puts "Something went wrong, delta #{delta} on gift_id #{gift_id}"
    elsif ( delta < -1e-6 )
      score += delta
      puts "#{i}: Gift #{gift_id} - delta #{delta}, score #{score}"
    end
  end

  solution = tc.to_solution
  solution.validate
  predicted_score = solution.score
  submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_twenty_#{predicted_score.to_i}.csv" )
  puts "Writing submission file: #{submission_file}"
  solution.write_csv( submission_file )

  break if (loop_start_score - predicted_score) < 1000.0
end