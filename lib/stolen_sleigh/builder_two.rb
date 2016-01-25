module StolenSleigh::Builder::Two
  def self.run
    raise NotImplementedError
    i = StolenSleigh.input
    tc = StolenSleigh::TripCollection.new
    score = 0.0

    loop do
      gift_id = i.furthest_gift
      break unless gift_id
      proto_trip = StolenSleigh::Builder::One.build_trip_for( gift_id, 0.3 )
      trip = StolenSleigh::Trip.from_narray( proto_trip )
      i.mark_as_used( trip.gifts )
      trip_id = tc.add_trip( trip )
      score += trip.score
      puts "Trip %4d: %4d gifts. Weight: %6.1f, score: %10.1f, efficiency: %7.4f. Total: %13.1f" % [trip_id, trip.num_gifts, trip.gifts_weight, trip.score, trip.efficiency, score]
    end
    tc.to_solution
  end
end
