class StolenSleigh::Solver
  attr_reader :opts, :solution, :score

  def initialize opts = {}
    set_opts( opts )
    @solution = StolenSleigh::Solution.new
  end

  def run
    tc = StolenSleigh::TripCollection.new
    (0..99999).each do |gift_id|
      trip = StolenSleigh::Trip.from_narray( NArray[ gift_id ] )
      tc.set_trip( gift_id, trip )
    end
    @solution = tc.to_solution
    @solution.validate
    @score = @solution.score
  end

  def self.default_opts
    Hash[
      :example => 0
    ]
  end

  def self.next_checkpoint current_score
    offset = if current_score > 18_000_000_000
      4_000_000_000
    elsif current_score > 15_000_000_000
      1_000_000_000
    elsif current_score > 13_000_000_000
      500_000_000
    elsif current_score > 13_000_000_000
      100_000_000
    elsif current_score > 12_500_000_000
      25_000_000
    elsif current_score > 12_450_000_000
      10_000_000
    elsif current_score > 12_410_000_000
       2_000_000
    elsif current_score > 12_400_000_000
         250_000
    else
          25_000
    end
    checkpoint = current_score - offset
    if checkpoint > 12_500_000_000
      return 12_500_000_000
    end
    checkpoint
  end

  private

  def set_opts opts
    @opts = self.class.default_opts.merge( opts )
  end
end

class StolenSleigh::TripCollection
  def pick_trips first_gift_picker = :random_gift, trip_group_size = 4, trip_closeness_metric = :haversine
    gift_id = case first_gift_picker
    when :random_gift then pick_first_gift_random
    when :sequence then pick_first_gift_sequence
    else
      raise "Don't recognise pick gift rule '#{first_gift_picker}'"
    end

    case trip_closeness_metric
    when :haversine
      return pick_trip_group_closest_n( gift_id, trip_group_size )
    when :long_only
      return pick_trip_group_closest_n_long_only( gift_id, trip_group_size )
    when :weighted
      return pick_trip_group_closest_n_weighted( gift_id, trip_group_size )
    else
      raise "Don't recognise trip_closeness_metric '#{trip_closeness_metric}'"
    end
  end

  def pick_first_gift_random
    (StolenSleigh.rand() * 100000).to_i
  end

  def pick_random_trip
    loop do
      trip_id = (StolenSleigh.rand() * (self.max_trip_id+1) ).to_i
      trip = self.get_trip( trip_id )
      break trip if trip
    end
  end

  def sampled_trips( num )
    all_trips = (0..self.max_trip_id).map { |id| self.get_trip( id ) }.compact.shuffle
    i = 0
    arr = []
    while (i < num)
      if ( i % self.num_trips ) == 0
        all_trips.shuffle!
      end
      trip = all_trips[ i % self.num_trips ]
      arr << trip.gifts.to_a.sample
      i += 1
    end
    arr
  end

  def pick_first_gift_sequence
    @gift_id ||= -1
    @gift_id += 1
    @gift_id = @gift_id % 100000
  end

  def pick_trip_group_closest_n gift_id, n = 4
    trip_ids = [ self.trip_id_of_gift( gift_id ) ]
    close_gift_ids = StolenSleigh.input.all_gifts_by_distance_from( gift_id )
    close_gift_ids.each do |next_gift_id|
      if next_trip_id = self.trip_id_of_gift( next_gift_id )
        trip_ids << next_trip_id
        trip_ids.uniq!
        break if trip_ids.length >= n
      end
    end
    trip_ids
  end

  def pick_trip_group_closest_n_long_only gift_id, n = 4
    trip_ids = [ self.trip_id_of_gift( gift_id ) ]
    close_gift_ids = StolenSleigh.input.all_gifts_by_long_only_distance_from( gift_id )
    close_gift_ids.each do |next_gift_id|
      if next_trip_id = self.trip_id_of_gift( next_gift_id )
        trip_ids << next_trip_id
        trip_ids.uniq!
        break if trip_ids.length >= n
      end
    end
    trip_ids
  end

  def pick_trip_group_closest_n_weighted gift_id, n = 4, w_hav = 0.01, w_long = 1.0, w_cyl = 2.0
    trip_ids = [ self.trip_id_of_gift( gift_id ) ]
    close_gift_ids = StolenSleigh.input.all_gifts_by_weighted_distance_from( gift_id, w_hav, w_long, w_cyl )
    close_gift_ids.each do |next_gift_id|
      if next_trip_id = self.trip_id_of_gift( next_gift_id )
        trip_ids << next_trip_id
        trip_ids.uniq!
        break if trip_ids.length >= n
      end
    end
    trip_ids
  end

  def pick_trips_close_to_trip trip, n = 30
    tg = trip.gifts.to_a.map do |gift_id|
      close_trip_ids = pick_trip_group_closest_n( gift_id, n )
    end
    (0...tg.count).flat_map { |idx| tg.map { |trip_array| trip_array[idx] } }.uniq[0...n]
  end
end
