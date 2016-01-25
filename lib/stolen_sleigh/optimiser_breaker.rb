module StolenSleigh::Optimiser::Breaker
  def self.adjust tc
    savings = 0.0
    num_trips = 0
    gift_ids = []

     (0..tc.max_trip_id).each do |trip_id|
      trip = tc.get_trip(trip_id)
      next unless trip
      pos,c = trip.lowest_cost_break_at
      next unless pos > 0

      num_trips += 1
      savings += c

      gift_ids << trip.gifts.to_a

      pre_gifts = trip.gifts[0...pos]
      post_gifts = trip.gifts[pos...trip.num_gifts]

      before_score = trip.score

      pre_trip = StolenSleigh::Trip.from_narray( pre_gifts )
      post_trip = StolenSleigh::Trip.from_narray( post_gifts )

      after_score = pre_trip.score + post_trip.score

      if ( after_score > before_score )
        raise "Something went wrong"
      end

      tc.set_trip( trip_id, pre_trip )
      new_trip_id = tc.add_trip( post_trip )

      puts "Split trip #{trip_id} into #{trip_id} & #{new_trip_id}, saving #{-c}"
    end

    if num_trips > 0
      puts "Total #{num_trips} split, saving #{-savings}"
    end
    gift_ids.flatten
  end
end
