require 'helpers'

describe StolenSleigh::Input do
  describe "class methods" do
    describe "#new" do
      it "creates a new object" do
        expect( StolenSleigh::Input.new ).to be_a StolenSleigh::Input
      end
    end

    describe "with Marshal" do
      before do
        @orig_input = StolenSleigh.input
        @saved_data = Marshal.dump( @orig_input )
        @copy_input =  Marshal.load( @saved_data )
      end

      it "can save and retrieve input data" do
        expect( @copy_input ).to_not be @orig_input
        expect( @copy_input.gifts ).to be_an NArray
        orig_gifts = @orig_input.gifts
        copy_gifts = @copy_input.gifts
        expect( copy_gifts ).to_not be orig_gifts
        expect( copy_gifts ).to be_narray_like orig_gifts
      end
    end
  end

  describe "instance methods" do
    before :each do
      @input = StolenSleigh.input
    end

    describe "#clone" do
      it "makes deep copy of input data" do
        @copy_input = @input.clone
        expect( @copy_input ).to_not be @input
        expect( @copy_input.gifts ).to be_an NArray
        orig_gifts = @input.gifts
        copy_gifts = @copy_input.gifts
        expect( copy_gifts ).to_not be orig_gifts
        expect( copy_gifts ).to be_narray_like orig_gifts
      end
    end

    describe "#sort_gifts_by_distance_from" do
      it "sorts an array of gift ids" do
        gifts = NArray[10,50,60,21,32,45,67,90]
        @input.sort_gifts_by_distance_from( 100, gifts )
        expect( gifts ).to be_narray_like NArray[ 50, 67, 21, 10, 45, 60, 90, 32 ]
        lat_100 = @input.gifts[0,100]
        lng_100 = @input.gifts[1,100]

        gifts.to_a.each_cons(2) do |closer_id,further_id|
          lat_closer = @input.gifts[0,closer_id]
          lng_closer = @input.gifts[1,closer_id]
          distance_a = StolenSleigh.haversine_distance( lat_100, lng_100, lat_closer, lng_closer )

          lat_further = @input.gifts[0,further_id]
          lng_further = @input.gifts[1,further_id]
          distance_b = StolenSleigh.haversine_distance( lat_100, lng_100, lat_further, lng_further )

          expect( distance_a ).to be < distance_b
        end
      end
    end

    describe "#all_gifts_by_distance_from" do
      it "returns an array of gifts progressively further from given id" do
        gifts = @input.all_gifts_by_distance_from( 12345 )
        expect( gifts ).to be_a NArray
        expect( gifts.size ).to eql 99999
        lat_12345 = @input.gifts[0,12345]
        lng_12345 = @input.gifts[1,12345]
        gifts.to_a.each_cons(2) do |closer_id,further_id|
          lat_closer = @input.gifts[0,closer_id]
          lng_closer = @input.gifts[1,closer_id]
          distance_a = StolenSleigh.haversine_distance( lat_12345, lng_12345, lat_closer, lng_closer )

          lat_further = @input.gifts[0,further_id]
          lng_further = @input.gifts[1,further_id]
          distance_b = StolenSleigh.haversine_distance( lat_12345, lng_12345, lat_further, lng_further )

          expect( distance_a ).to be < distance_b
        end
      end
    end

    describe "#all_gifts_by_long_only_distance_from" do
      it "returns an array of gifts progressively further from given id's longitude" do
        gifts = @input.all_gifts_by_long_only_distance_from( 12345 )
        expect( gifts ).to be_a NArray
        expect( gifts.size ).to eql 99999
        lat_12345 = @input.gifts[0,12345]
        lng_12345 = @input.gifts[1,12345]
        gifts.to_a.each_cons(2) do |closer_id,further_id|
          lat_closer = @input.gifts[0,closer_id]
          lng_closer = @input.gifts[1,closer_id]
          distance_a = StolenSleigh.haversine_distance( lat_closer, lng_12345, lat_closer, lng_closer )

          lat_further = @input.gifts[0,further_id]
          lng_further = @input.gifts[1,further_id]
          distance_b = StolenSleigh.haversine_distance( lat_further, lng_12345, lat_further, lng_further )

          expect( distance_a ).to be < distance_b
        end
      end
    end

    describe "#all_gifts_by_weighted_distance_from" do
      it "returns an array of gifts progressively further from a given id by the chosen metric" do
        gifts = @input.all_gifts_by_weighted_distance_from( 12345, 0.001, 1.0, 2.0 )
        expect( gifts ).to be_a NArray
        expect( gifts.size ).to eql 99999
        # Not sure how to test accuracy of metric
      end
    end

    describe "#all_gifts_within_bounds" do
      it "returns an array of gifts that satisfy given bounds on position" do
        gifts = @input.all_gifts_within_bounds( -1, 1, 0, 0.001 )
        expect( gifts.to_a.sort ).to eql [1073, 1362, 7409, 7761, 10230, 11313, 15241, 19131, 19418, 23414,
                                          27454, 28262, 29105, 33197, 33767, 34893, 36240, 41232, 42436, 48871,
                                          49308, 49943, 53917, 53936, 55239, 56136, 60833, 61468, 67978, 76722,
                                          78826, 83314, 87793, 93268, 94720, 95374, 97757, 98448, 99083, 99997]
        gifts.to_a.each do |gift_id|
          lat = @input.gifts[0,gift_id]
          lng = @input.gifts[1,gift_id]
          expect( lat ).to be > -1.0
          expect( lat ).to be < 1.0
          expect( lng ).to be > -0.001
          expect( lng ).to be < 0.001
        end
      end

      it "handles East/West wrap-around high to low" do
        gifts = @input.all_gifts_within_bounds( -1, 1, 3.12, 0.1 )
        gifts.to_a.each do |gift_id|
          lat = @input.gifts[0,gift_id]
          lng = @input.gifts[1,gift_id]
          expect( lat ).to be > -1.0
          expect( lat ).to be < 1.0
          if lng > 1
            expect( lng ).to be > 3.02
            expect( lng ).to be < Math::PI
          else
            expect( lng ).to be < ( 3.22 - 2 * Math::PI )
            expect( lng ).to be > ( - Math::PI )
          end
        end
      end

      it "handles East/West wrap-around low to high" do
        gifts = @input.all_gifts_within_bounds( -1, 1, -3.12, 0.1 )
        gifts.to_a.each do |gift_id|
          lat = @input.gifts[0,gift_id]
          lng = @input.gifts[1,gift_id]
          expect( lat ).to be > -1.0
          expect( lat ).to be < 1.0
          if lng > 1
            expect( lng ).to be > ( 2 * Math::PI - 3.22 )
            expect( lng ).to be < Math::PI
          else
            expect( lng ).to be < -3.02
            expect( lng ).to be > ( - Math::PI )
          end
        end
      end
    end

    describe "#most_costly_gift" do
      it "returns the gift with highest individual cost to deliver" do
        expect( @input.most_costly_gift ).to be 24692
        costs = StolenSleigh.input.gifts[4,0..99999]
        expect( StolenSleigh.input.gifts[4,24692] ).to be_within(1e-6).of costs.max
      end

      it "returns the next highest if the highest has been used" do
        @input.mark_as_used( NArray[24692] )
        expect( @input.most_costly_gift ).to be 56837
        costs = StolenSleigh.input.gifts[4,0..99999]
        costs[24692] = 0.0
        expect( StolenSleigh.input.gifts[4,56837] ).to be_within(1e-6).of costs.max
      end
    end

    describe "#heaviest_gift" do
      it "returns a gift weighing 50.0" do
        gift_id = @input.heaviest_gift
        weight = StolenSleigh.input.gifts[2,gift_id]
        expect( weight ).to be_within(1e-6).of 50.0
      end
    end
  end

  describe "imported data" do
    before :each do
      @input = StolenSleigh.input
      @gifts = @input.gifts
    end

    it "has all angles within bounds" do
      lattitudes = @gifts[0,0..99999].to_a
      min_lat = -Math::PI/2
      max_lat = Math::PI/2
      expect( lattitudes.all? { |l| l >= min_lat && l <= max_lat } ).to be true

      longitudes = @gifts[1,0..99999].to_a
      min_lng = -Math::PI
      max_lng = Math::PI
      expect( longitudes.all? { |l| l >= min_lng && l <= max_lng } ).to be true
    end

    it "has all weights within bounds" do
      weights = @gifts[2,0..99999].to_a
      min_weight = 1.0
      max_weight = 990.0
      expect( weights.all? { |l| l >= min_weight && l <= max_weight } ).to be true
    end

    it "has expected total weariness for 100,000 single journeys" do
      expect( @gifts[4,0..99999].sum ).to be_within(0.1).of 29121011015.6
    end
  end
end
