module StolenSleigh::Optimiser
  def self.optimise_random_trip_arrays solution_file, output_name, num_trips, max_reps = 100000, trip_closeness_metric = :haversine
    s = StolenSleigh::Solution.new
    s.import_csv( solution_file )
    tc = StolenSleigh::TripCollection.from_solution( s )
    predicted_score = tc.score
    target_score = StolenSleigh::Solver.next_checkpoint( predicted_score )

    max_reps.times do |i|
      trip_ids = tc.pick_trips( :random_gift, num_trips, trip_closeness_metric )

      trips = trip_ids.map {|id| tc.get_trip(id) }
      group_score = trips.inject(0.0) {|t,trip| t + trip.score }

      trips.push StolenSleigh::Trip.new(0)
      trip_ids.push -1

      new_trips, new_score = yield( trips )

      if ( new_score < group_score )
        trip_ids.zip( new_trips ).each do |trip_id, new_trip|
          if new_trip.gifts.empty?
            if ( trip_id > -1 )
              puts "  deleting trip #{trip_id}!"
              tc.delete_trip( trip_id )
            end
          else
            if ( trip_id > -1 )
              tc.set_trip( trip_id, new_trip )
            else
              trip_id =  tc.add_trip( new_trip )
              puts "  adding trip #{trip_id}!"
            end
          end
        end

        predicted_score += new_score - group_score
        puts "#{i} - #{predicted_score}"

        if ( predicted_score < target_score )
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          target_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      else
        puts "#{i} -"
      end

      if (i % opts[:adjust_every] ) == 0
        StolenSleigh::Optimiser::Breaker.adjust( tc )
      end
    end

    new_solution = tc.to_solution
    new_solution.validate
    this_score = new_solution.score
    submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
    puts "Completed. Writing submission file: #{submission_file}"
    new_solution.write_csv( submission_file )
  end

  def self.shaker solution_file, output_name, opts = self.default_opts
    opts = self.default_opts.merge( opts )

    s = StolenSleigh::Solution.new
    s.import_csv( solution_file )
    tc = StolenSleigh::TripCollection.from_solution( s )
    predicted_score = tc.score
    target_score = StolenSleigh::Solver.next_checkpoint( predicted_score )
    seen_gifts = Hash.new

    if opts[:gift_choice] == :shuffle_trips
      gift_array = tc.sampled_trips( opts[:outer_reps] )
    end

    opts[:outer_reps].times do |i|
      if has_dynamic_opts?( opts )
        dynamic_opts_calc( i, opts )
      end

      gift_id = case opts[:gift_choice]
      when :random then tc.pick_first_gift_random
      when :shuffle then
        shuffled_ids ||= (0..99999).to_a.shuffle
        shuffled_pos ||= -1
        shuffled_pos = (shuffled_pos + 1) % 100000
        shuffled_ids[ shuffled_pos ]
      when :tension then
        tensions = tc.gift_tensions
        tsi = tensions.sort_index.to_a.reverse
        idx = tsi.find { |id| !seen_gifts[id] }
        seen_gifts[idx] = true
        idx
      when :shuffle_trips then
        gift_array[i]
      end
      trip_ids = tc.pick_trip_group_closest_n_weighted( gift_id, opts[:num_trips], opts[:w_hav], opts[:w_long], opts[:w_cyl] )

      trips = trip_ids.map {|id| tc.get_trip(id) }
      group_score = trips.inject(0.0) {|t,trip| t + trip.score }

      trips.push StolenSleigh::Trip.new(0)
      trip_ids.push -1

      new_trips, new_score = StolenSleigh::Optimiser::Shaker.multi_trip_shake( trips, opts[:inner_reps], opts[:draw] )

      if ( new_score < group_score )
        trip_ids.zip( new_trips ).each do |trip_id, new_trip|
          if new_trip.gifts.empty?
            if ( trip_id > -1 )
              puts "  deleting trip #{trip_id}!"
              tc.delete_trip( trip_id )
            end
          else
            if ( trip_id > -1 )
              tc.set_trip( trip_id, new_trip )
            else
              trip_id =  tc.add_trip( new_trip )
              puts "  adding trip #{trip_id}!"
            end
          end
        end

        predicted_score += new_score - group_score
        puts "#{i} - #{predicted_score}"

        if ( predicted_score < target_score )
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          target_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      else
        puts "#{i} -"
      end

      if (i % opts[:adjust_every] ) == 0
        StolenSleigh::Optimiser::Breaker.adjust( tc )
        predicted_score = tc.score
      end
    end

    StolenSleigh::Optimiser::Breaker.adjust( tc )

    new_solution = tc.to_solution
    new_solution.validate
    this_score = new_solution.score
    submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
    puts "Completed. Writing submission file: #{submission_file}"
    new_solution.write_csv( submission_file )
  end

  def self.shaker_sa solution_file, output_name, opts = self.default_opts
    opts = self.default_opts.merge( opts )

    s = StolenSleigh::Solution.new
    s.import_csv( solution_file )
    tc = StolenSleigh::TripCollection.from_solution( s )
    StolenSleigh::Optimiser::Breaker.adjust( tc )

    predicted_score = tc.score
    target_score = StolenSleigh::Solver.next_checkpoint( predicted_score )
    seen_gifts = Hash.new

    if opts[:gift_choice] == :shuffle_trips
      gift_array = tc.sampled_trips( opts[:outer_reps] )
    end

    opts[:outer_reps].times do |i|
      gift_id = case opts[:gift_choice]
      when :random then tc.pick_first_gift_random
      when :shuffle then
        shuffled_ids ||= (0..99999).to_a.shuffle
        shuffled_pos ||= -1
        shuffled_pos = (shuffled_pos + 1) % 100000
        shuffled_ids[ shuffled_pos ]
      when :tension then
        tensions = tc.gift_tensions
        tsi = tensions.sort_index.to_a.reverse
        idx = tsi.find { |id| !seen_gifts[id] }
        seen_gifts[idx] = true
        idx
      when :shuffle_trips then
        gift_array[i]
      end

      trip_ids = tc.pick_trip_group_closest_n_weighted( gift_id, opts[:num_trips], opts[:w_hav], opts[:w_long], opts[:w_cyl] )

      trips = trip_ids.map {|id| tc.get_trip(id) }
      group_score = trips.inject(0.0) {|t,trip| t + trip.score }

      trips.push StolenSleigh::Trip.new(0)
      trip_ids.push -1

      new_trips, new_score = StolenSleigh::Optimiser::Shaker.multi_trip_shake_sa(
          trips, opts[:inner_reps], opts[:draw], opts[:start_t], opts[:end_t] )

      if ( new_score < group_score )
        trip_ids.zip( new_trips ).each do |trip_id, new_trip|
          if new_trip.gifts.empty?
            if ( trip_id > -1 )
              puts "  deleting trip #{trip_id}!"
              tc.delete_trip( trip_id )
            end
          else
            if ( trip_id > -1 )
              tc.set_trip( trip_id, new_trip )
            else
              trip_id =  tc.add_trip( new_trip )
              puts "  adding trip #{trip_id}!"
            end
          end
        end

        predicted_score += new_score - group_score
        puts "#{i} - #{predicted_score}"

        if ( predicted_score < target_score )
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          target_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      else
        puts "#{i} -"
      end

      if (i % opts[:adjust_every] ) == 0
        StolenSleigh::Optimiser::Breaker.adjust( tc )
        predicted_score = tc.score
      end
    end

    StolenSleigh::Optimiser::Breaker.adjust( tc )

    new_solution = tc.to_solution
    new_solution.validate
    this_score = new_solution.score
    submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
    puts "Completed. Writing submission file: #{submission_file}"
    new_solution.write_csv( submission_file )
  end


  def self.shaker2_sa solution_file, output_name, opts = self.default_opts
    opts = self.default_opts.merge( opts )

    s = StolenSleigh::Solution.new
    s.import_csv( solution_file )
    tc = StolenSleigh::TripCollection.from_solution( s )
    StolenSleigh::Optimiser::Breaker.adjust( tc )

    predicted_score = tc.score
    target_score = StolenSleigh::Solver.next_checkpoint( predicted_score )
    seen_gifts = Hash.new

    opts[:outer_reps].times do |i|
      gift_id = case opts[:gift_choice]
      when :random then tc.pick_first_gift_random
      when :shuffle then
        shuffled_ids ||= (0..99999).to_a.shuffle
        shuffled_pos ||= -1
        shuffled_pos = (shuffled_pos + 1) % 100000
        shuffled_ids[ shuffled_pos ]
      when :tension then
        tensions = tc.gift_tensions
        tsi = tensions.sort_index.to_a.reverse
        idx = tsi.find { |id| !seen_gifts[id] }
        seen_gifts[idx] = true
        idx
      end

      trip_ids = tc.pick_trip_group_closest_n_weighted( gift_id, opts[:num_trips], opts[:w_hav], opts[:w_long], opts[:w_cyl] )

      trips = trip_ids.map {|id| tc.get_trip(id) }
      group_score = trips.inject(0.0) {|t,trip| t + trip.score }

      trips.push StolenSleigh::Trip.new(0)
      trip_ids.push -1

      new_trips, new_score = StolenSleigh::Optimiser::Shaker.simple_shake_sa(
          trips, opts[:inner_reps], opts[:draw], opts[:start_t], opts[:end_t] )

      if ( new_score < group_score )
        trip_ids.zip( new_trips ).each do |trip_id, new_trip|
          if new_trip.gifts.empty?
            if ( trip_id > -1 )
              puts "  deleting trip #{trip_id}!"
              tc.delete_trip( trip_id )
            end
          else
            if ( trip_id > -1 )
              tc.set_trip( trip_id, new_trip )
            else
              trip_id =  tc.add_trip( new_trip )
              puts "  adding trip #{trip_id}!"
            end
          end
        end

        predicted_score += new_score - group_score
        puts "#{i} - #{predicted_score}"

        if ( predicted_score < target_score )
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          target_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      else
        puts "#{i} -"
      end

      if (i % opts[:adjust_every] ) == 0
        StolenSleigh::Optimiser::Breaker.adjust( tc )
        predicted_score = tc.score
      end
    end

    StolenSleigh::Optimiser::Breaker.adjust( tc )

    new_solution = tc.to_solution
    new_solution.validate
    this_score = new_solution.score
    submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
    puts "Completed. Writing submission file: #{submission_file}"
    new_solution.write_csv( submission_file )
  end

  def self.splicer_sa solution_file, output_name, opts = self.default_opts
    opts = self.default_opts.merge( opts )

    s = StolenSleigh::Solution.new
    s.import_csv( solution_file )
    tc = StolenSleigh::TripCollection.from_solution( s )
    StolenSleigh::Optimiser::Breaker.adjust( tc )

    predicted_score = tc.score
    target_score = StolenSleigh::Solver.next_checkpoint( predicted_score )
    seen_gifts = Hash.new

    opts[:outer_reps].times do |i|
      gift_id = case opts[:gift_choice]
      when :random then tc.pick_first_gift_random
      when :shuffle then
        shuffled_ids ||= (0..99999).to_a.shuffle
        shuffled_pos ||= -1
        shuffled_pos = (shuffled_pos + 1) % 100000
        shuffled_ids[ shuffled_pos ]
      when :tension then
        tensions = tc.gift_tensions
        tsi = tensions.sort_index.to_a.reverse
        idx = tsi.find { |id| !seen_gifts[id] }
        seen_gifts[idx] = true
        idx
      end

      trip_ids = tc.pick_trip_group_closest_n_weighted( gift_id, opts[:num_trips], opts[:w_hav], opts[:w_long], opts[:w_cyl] )

      trips = trip_ids.map {|id| tc.get_trip(id) }
      group_score = trips.inject(0.0) {|t,trip| t + trip.score }

      trips.push StolenSleigh::Trip.new(0)
      trip_ids.push -1

      new_trips, new_score = StolenSleigh::Optimiser::Splicer.multi_trip_splice_sa(
          trips, opts[:inner_reps], opts[:draw], opts[:start_t], opts[:end_t] )

      if ( new_score < group_score )
        trip_ids.zip( new_trips ).each do |trip_id, new_trip|
          if new_trip.gifts.empty?
            if ( trip_id > -1 )
              puts "  deleting trip #{trip_id}!"
              tc.delete_trip( trip_id )
            end
          else
            if ( trip_id > -1 )
              tc.set_trip( trip_id, new_trip )
            else
              trip_id =  tc.add_trip( new_trip )
              puts "  adding trip #{trip_id}!"
            end
          end
        end

        predicted_score += new_score - group_score
        puts "#{i} - #{predicted_score}"

        if ( predicted_score < target_score )
          new_solution = tc.to_solution
          new_solution.validate
          this_score = new_solution.score
          submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
          puts "Writing submission file: #{submission_file}"
          new_solution.write_csv( submission_file )
          target_score = StolenSleigh::Solver.next_checkpoint( this_score )
        end
      else
        puts "#{i} - [#{group_score}, #{new_score}]"
      end

      if (i % opts[:adjust_every] ) == 0
        StolenSleigh::Optimiser::Breaker.adjust( tc )
        predicted_score = tc.score
      end
    end

    StolenSleigh::Optimiser::Breaker.adjust( tc )

    new_solution = tc.to_solution
    new_solution.validate
    this_score = new_solution.score
    submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{output_name}_#{this_score.to_i}.csv" )
    puts "Completed. Writing submission file: #{submission_file}"
    new_solution.write_csv( submission_file )
  end

  def self.has_dynamic_opts? opts
    opts[:dynamic]
  end

  def self.dynamic_opts_calc current_rep, opts
    ratio = current_rep.to_f/opts[:outer_reps]
    if opts[:dyn_num_trips]
      opts[:num_trips] = ( (1.0-ratio) * opts[:dyn_num_trips].first + ratio * opts[:dyn_num_trips].last ).to_i
    end
    if opts[:dyn_draw]
      opts[:draw] = ( (1.0-ratio) * opts[:dyn_draw].first + ratio * opts[:dyn_draw].last ).to_i
    end
    if opts[:dyn_inner_reps]
      opts[:inner_reps] = ( (1.0-ratio) * opts[:dyn_inner_reps].first + ratio * opts[:dyn_inner_reps].last ).to_i
    end
  end

  def self.default_opts
    Hash[ :dynamic => false,
      :num_trips => 6, :inner_reps => 1000, :draw => 3, :outer_reps => 10000,
      :start_t => 500, :end_t => 10, :gift_choice => :random, :adjust_every => 100000,
      :num_trips => 6, :w_hav => 0.05, :w_long => 1.0, :w_cyl => 1.0
    ]
  end

end
