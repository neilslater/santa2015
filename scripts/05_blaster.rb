require 'stolen_sleigh'

srand( 27101283321245365044 )

s = StolenSleigh::Solution.new

s.import_csv('output/submission_best_12411153279.csv')

$tc = StolenSleigh::TripCollection.from_solution( s )

score = $tc.score

trip_sizes = Hash[ (0..$tc.max_trip_id).map { |id| if trip = $tc.get_trip(id); [id,trip.num_gifts]; else nil; end }.compact ]

trip_sizes.sort_by {|k,v| v}.each do |trip_id,trip_size|
  puts "#{trip_id}  (#{trip_size} gifts)"

  trip = $tc.get_trip( trip_id )

  before_score = $tc.score
  backup_tc = $tc.clone

  $tc.delete_trip( trip_id )

  ok = trip.gifts.to_a.each do |gift_id|
    break false unless $tc.insert_gift_at_lowest_cost( gift_id )
  end
  ok = !! ok

  delta = $tc.score - before_score

  if (ok && delta < -1.0)
    puts " Improved: #{$tc.score} (was #{before_score})"
    #solution = $tc.to_solution
    #solution.validate
  elsif
    $tc = backup_tc
    puts " No improvement: #{$tc.score} (was #{before_score})"
    #solution = $tc.to_solution
    #solution.validate
  end
end

solution = $tc.to_solution
solution.validate
predicted_score = solution.score
submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_twenty_#{predicted_score.to_i}.csv" )
puts "Writing submission file: #{submission_file}"
solution.write_csv( submission_file )
