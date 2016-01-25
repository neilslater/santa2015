require 'stolen_sleigh'

StolenSleigh.srand_array( [64437102,5232312,4343414,8834255,91343241,83435634,844431354,99342843,8761665] )

s = StolenSleigh::Solution.new

s.import_csv('output/submission_twenty_12410320806.csv')

tc = StolenSleigh::TripCollection.from_solution( s )

predicted_score = tc.score

gift_tensions = tc.gift_tensions

trip_tensions = Hash[ (0..tc.max_trip_id).map { |id| if trip = tc.get_trip(id); [id,trip.total_tension]; else nil; end }.compact ]

trip_tensions.sort_by { |k,v| v }.reverse.each do |trip_id, tension|

puts "- - - - - #{trip_id}, tension: #{tension}"

trip = tc.get_trip( trip_id )

# Just pick the middle gift
gift_id = trip.gifts.to_a[ trip.gifts.size/2 ]

trip_ids_to_opt = tc.pick_trip_group_closest_n_weighted( gift_id, 70, 0.025, 0.85, 0.15 )

trips_to_opt = Hash[ trip_ids_to_opt.map { |trip_id| [ trip_id, tc.get_trip( trip_id ) ] } ]

trips_to_opt_score = trips_to_opt.values.inject(0.0) { |score,trip| score + trip.score }

################################################################################################
# Create new "split" trips
split_trip_a = StolenSleigh::Trip.new(0)
split_trip_b = StolenSleigh::Trip.new(0)
empty_trip = StolenSleigh::Trip.new(0)

gifts_to_split = trip.gifts.to_a
long_of_gifts = Hash[ gifts_to_split.map { |gid| [gid,StolenSleigh.input.gifts[1,gid]] } ]

# TODO: Split point should median? Or even more complex?
split_point = long_of_gifts.values.sort[ long_of_gifts.values.count/2 ]

long_of_gifts.each do |gift_id,gift_long|
  if gift_long > split_point
    split_trip_a.add_gift(  split_trip_a.lowest_cost_insert_at( gift_id ).first, gift_id )
  else
    split_trip_b.add_gift(  split_trip_b.lowest_cost_insert_at( gift_id ).first, gift_id )
  end
end

trips_to_opt[trip_id] = split_trip_a
trips_to_opt[-1] = split_trip_b
trips_to_opt[-2] = empty_trip

replacement_trips = trips_to_opt.values

replacement_trips_score = replacement_trips.inject(0.0) { |score,trip| score + trip.score }

puts "Before score: #{trips_to_opt_score}"
puts "Middle score: #{replacement_trips_score}"

# new_trips, new_score = StolenSleigh::Optimiser::Shaker.multi_trip_shake_sa( replacement_trips, 30000, 3, 5000, 50 )
new_trips, new_score = StolenSleigh::Optimiser::Shaker.multi_trip_shake( replacement_trips, 3000, 3 )

puts "After score: #{new_score}"

saved = trips_to_opt_score-new_score;

if saved > 0
  puts "   SAVED: #{saved} !"
  num_trips = replacement_trips.count { |trip| trip.num_gifts > 0 }
  puts "   #{num_trips} populated trips"
  puts "   original trip_ids [#{trips_to_opt.keys.join(', ')}]"

  trips_to_opt.keys.zip(new_trips).each do |trip_id, trip|
    if trip_id >= 0
      if ( trip.gifts.empty? )
        tc.remove_trip( trip_id )
        puts "Removed trip #{trip_id}"
      else
        tc.set_trip( trip_id, trip )
      end
    else
      if ( ! trip.gifts.empty? )
        new_trip_id = tc.add_trip( trip )
        puts "Added trip #{new_trip_id}"
      end
    end
  end

  new_solution = tc.to_solution
  new_solution.validate
  this_score = new_solution.score
  submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_twenty_#{this_score.to_i}.csv" )
  puts "Writing submission file: #{submission_file}"
  new_solution.write_csv( submission_file )
end

end
