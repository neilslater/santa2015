require 'helpers'

describe StolenSleigh::TripCollection do
  describe "class methods" do
    describe "#new" do
      it "creates a new object" do
        expect( StolenSleigh::TripCollection.new ).to be_a StolenSleigh::TripCollection
      end

      it "creates an empty collection" do
        trip_collection = StolenSleigh::TripCollection.new
        expect( trip_collection.num_trips ).to eql 0
        expect( trip_collection.get_trip(0) ).to be_nil
        expect( trip_collection.max_trip_id ).to eql -1
      end
    end
  end

  describe "instance methods" do
    before :each do
      @tc = StolenSleigh::TripCollection.new
    end

    describe "#add_trip" do
      it "increases num_trips" do
        trip = StolenSleigh::Trip.new(1)
        trip.gifts[0] = 1
        trip_id = @tc.add_trip( trip )
        expect( @tc.num_trips ).to eql 1
        expect( @tc.max_trip_id ).to eql 0
      end

      it "stores a retrievable copy of the trip" do
        trip = StolenSleigh::Trip.new(1)
        trip.gifts[0] = 17
        trip_id = @tc.add_trip( trip )

        retrieved_trip = @tc.get_trip( trip_id )
        expect( retrieved_trip ).to be_a StolenSleigh::Trip
        expect( retrieved_trip ).to_not be trip
        expect( retrieved_trip.gifts ).to be_narray_like trip.gifts
      end

      it "can be repeated, adding items to end of collection" do
        [7,800,90123].each do |gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.add_trip( trip )
        end

        [7,800,90123].each_with_index do |gift_id,idx|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end
      end
    end

    describe "#set_trip" do
      it "increases num_trips" do
        trip = StolenSleigh::Trip.new(1)
        trip.set_gift( 0, 1 )
        trip_id = @tc.set_trip( 7, trip )
        expect( trip_id ).to eql 7
        expect( @tc.num_trips ).to eql 1
        expect( @tc.max_trip_id ).to eql 7
      end

      it "stores a retrievable copy of the trip" do
        trip = StolenSleigh::Trip.new(1)
        trip.set_gift( 0, 12312 )
        @tc.set_trip( 7, trip )

        retrieved_trip = @tc.get_trip( 7 )
        expect( retrieved_trip ).to be_a StolenSleigh::Trip
        expect( retrieved_trip ).to_not be trip
        expect( retrieved_trip.gifts ).to be_narray_like trip.gifts
      end

      it "can be repeated, adding items to end of collection" do
        [ [7,23], [800, 94], [90123,12] ].each do |idx,gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.set_trip( idx, trip )
        end

        [ [7,23], [800, 94], [90123,12] ].each do |idx,gift_id|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end

        expect( @tc.num_trips ).to eql 3
        expect( @tc.max_trip_id ).to eql 90123
      end

      it "over-writes previous values" do
        [ [7,23], [800, 94], [90123,12] ].each do |idx,gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.set_trip( idx, trip )
        end

        trip = StolenSleigh::Trip.new(2)
        trip.set_gift( 0, 55 )
        trip.set_gift( 1, 76543 )
        @tc.set_trip( 800, trip )

        [ [7,23], [800, 55], [90123,12] ].each do |idx,gift_id|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end

        expect( @tc.get_trip( 800 ).gifts[1] ).to eql 76543
      end
    end

    describe "#get_trip" do
      it "is nil by default" do
        expect( @tc.get_trip( 50 ) ).to be_nil
      end
    end

    describe "#delete_trip" do
      before :each do
        [ [7,23], [800, 94], [90123,12] ].each do |idx,gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.set_trip( idx, trip )
        end
      end

      it "has no effect when using unallocated idx value" do
        expect( @tc.delete_trip( 47613 ) ).to be false

         [ [7,23], [800, 94], [90123,12] ].each do |idx,gift_id|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end

        expect( @tc.num_trips ).to eql 3
        expect( @tc.max_trip_id ).to eql 90123
      end

      it "removes item at specified idx value" do
        expect( @tc.delete_trip( 800 ) ).to be true

         [ [7,23], [90123,12] ].each do |idx,gift_id|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end

        expect( @tc.get_trip( 800 ) ).to be_nil

        expect( @tc.num_trips ).to eql 2
        expect( @tc.max_trip_id ).to eql 90123
      end

      it "adjusts max_trip_id" do
        expect( @tc.delete_trip( 90123 ) ).to be true

        [ [7,23], [800, 94] ].each do |idx,gift_id|
          retrieved_trip = @tc.get_trip( idx )
          expect( retrieved_trip ).to be_a StolenSleigh::Trip
          expect( retrieved_trip.gifts[0] ).to eql gift_id
        end

        expect( @tc.get_trip( 90123 ) ).to be_nil

        expect( @tc.num_trips ).to eql 2
        expect( @tc.max_trip_id ).to eql 800
      end
    end

    describe "#score" do
      it "is 0.0 for empty collection" do
        expect( @tc.score ).to eql 0.0
      end

      it "is 29121011015.6 for one trip per item" do
        (0..99999).each do |gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.set_trip( gift_id, trip )
        end
        expect( @tc.score ).to be_within(0.1).of 29121011015.6
      end
    end

    describe "#to_solution" do
      it "creates a valid solution from completed trip data" do
        (0..99999).each do |gift_id|
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          @tc.set_trip( gift_id, trip )
        end
        solution = @tc.to_solution

        expect( solution ).to be_a StolenSleigh::Solution
        expect( solution.validate ).to be true
      end
    end

    describe "#trip_id_of_gift" do
      it "finds which trip a gift was assigned to" do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end

        expect( @tc.trip_id_of_gift( 78 ) ).to eql 78
        expect( @tc.trip_id_of_gift( 212 ) ).to eql 12
        expect( @tc.trip_id_of_gift( 112 ) ).to be_nil
      end

      it "re-indexes for new trip additions" do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end

        expect( @tc.trip_id_of_gift( 78 ) ).to eql 78
        expect( @tc.trip_id_of_gift( 212 ) ).to eql 12
        expect( @tc.trip_id_of_gift( 112 ) ).to be_nil
        trip = StolenSleigh::Trip.new(2)
        trip.set_gift( 0, 112 )
        trip.set_gift( 1, 110 )

        @tc.set_trip( 12, trip )

        expect( @tc.trip_id_of_gift( 78 ) ).to eql 78
        expect( @tc.trip_id_of_gift( 212 ) ).to be_nil
        expect( @tc.trip_id_of_gift( 112 ) ).to eql 12
        expect( @tc.trip_id_of_gift( 110 ) ).to eql 12
      end

      it "re-indexes for trip deletions" do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end

        expect( @tc.trip_id_of_gift( 78 ) ).to eql 78
        expect( @tc.trip_id_of_gift( 212 ) ).to eql 12
        expect( @tc.trip_id_of_gift( 112 ) ).to be_nil

        @tc.delete_trip( 12 )

        expect( @tc.trip_id_of_gift( 78 ) ).to eql 78
        expect( @tc.trip_id_of_gift( 12 ) ).to be_nil
        expect( @tc.trip_id_of_gift( 212 ) ).to be_nil
        expect( @tc.trip_id_of_gift( 112 ) ).to be_nil
      end
    end

    describe "#pick_trip_group_closest_n" do
      before :each do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end
      end

      it "finds four nearby trips by default" do
        expect( @tc.pick_trip_group_closest_n( 17 ) ).to eql [17, 68, 83, 42]
      end

      it "finds any number of nearby trips" do
        expect( @tc.pick_trip_group_closest_n( 17, 8 ) ).to eql [17, 68, 83, 42, 31, 51, 9, 59]
      end
    end

    describe "#insert_gift_at_lowest_cost" do
      before :each do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end
      end

      it "returns cost of making insert" do
        expect( @tc.insert_gift_at_lowest_cost( 400 ) ).to be_within(1e-6).of 20175.820272
      end

      it "assigns the gift to a trip" do
        @tc.insert_gift_at_lowest_cost( 400 )
        # This trip verified lowest cost outside of spec
        expect( @tc.trip_id_of_gift( 400 ) ).to eql 75
      end

      it "ends up with correct adjusted score" do
        before_score = @tc.score
        @tc.insert_gift_at_lowest_cost( 400 )
        after_score = @tc.score
        expect( after_score - before_score ).to be_within(1e-6).of 20175.820272
      end
    end

    describe "#gift_tensions" do
      before :each do
        (0..100).each do |gift_id|
          trip = StolenSleigh::Trip.new(2)
          trip.set_gift( 0, gift_id )
          trip.set_gift( 1, gift_id + 200 )
          @tc.set_trip( gift_id, trip )
        end
        @tensions = @tc.gift_tensions
      end

      it "should be a NArray" do
        expect( @tensions ).to be_a NArray
      end

      it "should have 0.0 rating for missing gifts" do
        expect( @tensions[107] ).to eql 0.0
      end

      it "should have 0.0 rating for start gifts" do
        (0..100).each do |gift_id|
          expect( @tensions[gift_id] ).to eql 0.0
        end
      end

      it "should have greater than 0.0 rating for second, onwards gifts" do
        (200..300).each do |gift_id|
          expect( @tensions[gift_id] ).to be > 0.0
        end
      end
    end
  end
end
