require 'helpers'

describe StolenSleigh::Trip do
  describe "class methods" do
    describe "#new" do
      it "creates a new object" do
        expect( StolenSleigh::Trip.new(5) ).to be_a StolenSleigh::Trip
      end
    end

    describe "with Marshal" do
      before do
        @orig_trip = StolenSleigh::Trip.new(3)
        (0..2).each do |i|
          @orig_trip.set_gift( i, i )
        end
        @saved_data = Marshal.dump( @orig_trip )
        @copy_trip =  Marshal.load( @saved_data )
      end

      it "can save and retrieve training data" do
        expect( @copy_trip ).to_not be @orig_trip
        expect( @copy_trip.gifts ).to be_an NArray
        orig_gifts = @orig_trip.gifts
        copy_gifts = @copy_trip.gifts
        expect( copy_gifts ).to_not be orig_gifts
        expect( copy_gifts ).to be_narray_like orig_gifts
      end
    end
  end

  describe "instance methods" do
    before :each do
      @trip = StolenSleigh::Trip.new(3)
      (0..2).each do |i|
        @trip.set_gift( i, i + 10 )
      end
      @all_gifts = StolenSleigh.input.gifts
    end

    describe "#clone" do
      it "makes deep copy of trip data" do
        @copy_trip = @trip.clone
        expect( @copy_trip ).to_not be @trip
        expect( @copy_trip.gifts ).to be_an NArray
        orig_gifts = @trip.gifts
        copy_gifts = @copy_trip.gifts
        expect( copy_gifts ).to_not be orig_gifts
        expect( copy_gifts ).to be_narray_like orig_gifts
      end
    end

    describe "#gifts_weight" do
      it "is zero for an empty trip" do
        trip = StolenSleigh::Trip.new(0)
        expect( trip.gifts_weight ).to eql 0.0
      end

      it "equals the gift weight when there is one gift to deliver" do
        (0..100).each do |test_id|
          gift_id = test_id * 50 + 8
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          expect( trip.gifts_weight ).to be_within(1e-6).of @all_gifts[2,gift_id]
        end
      end

      it "works for a longer trip" do
        trip = StolenSleigh::Trip.new(100)
        (0..99).each do |test_id|
          gift_id = test_id * 50 + 8
          trip.set_gift( test_id, gift_id )
        end
        expect( trip.gifts_weight ).to be_within(1e-6).of 1541.822961
      end
    end

    describe "#score" do
      it "is zero for an empty trip" do
        trip = StolenSleigh::Trip.new(0)
        expect( trip.score ).to eql 0.0
      end

      it "equals the gift single score when there is one gift to deliver" do
        (0..100).each do |test_id|
          gift_id = test_id * 50 + 3
          trip = StolenSleigh::Trip.new(1)
          trip.set_gift( 0, gift_id )
          expect( trip.score ).to be_within(1e-6).of @all_gifts[4,gift_id]
        end
      end

      it "works for a longer trip" do
        trip = StolenSleigh::Trip.new(100)
        (0..99).each do |test_id|
          gift_id = test_id * 50 + 3
          trip.set_gift( test_id, gift_id )
        end
        expect( trip.score ).to be_within(1e-6).of 586995902.9664949
      end
    end

    describe "#efficiency" do
      it "is zero for an empty trip" do
        trip = StolenSleigh::Trip.new(0)
        expect( trip.efficiency ).to eql 0.0
      end

      it "equals 1.0 for a single gift" do
        trip = StolenSleigh::Trip.new(1)
        trip.set_gift( 0, 7 )
        expect( trip.efficiency ).to eql 1.0
      end

      it "works for a longer trip" do
        trip = StolenSleigh::Trip.new(100)
        (0..99).each do |test_id|
          gift_id = test_id * 50 + 3
          trip.set_gift( test_id, gift_id )
        end
        expect( trip.efficiency ).to be_within(1e-6).of 0.0495209
      end

      it "is higher for a well-chosen combination" do
        gifts = NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069]
        trip = StolenSleigh::Trip.from_narray( gifts )
        expect( trip.efficiency ).to be_within(1e-6).of 1.847171
      end
    end

    describe "#add_gift" do
      context "adding to start" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069] )
          @added_trip = StolenSleigh::Trip.new(0)
          [ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069].reverse.each do |gift_id|
            @added_trip.add_gift( 0, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "adding to end" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
          @added_trip = StolenSleigh::Trip.new(0)
          [ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ].each_with_index do |gift_id,idx|
            @added_trip.add_gift( idx, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "adding to middle (1)" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
          @added_trip = StolenSleigh::Trip.new(0)
          @added_trip.add_gift( 0, 69913 )
          @added_trip.add_gift( 1,  21069 )

          [ 31517, 51457, 36237, 74932, 58021, 93699 ].reverse.each do |gift_id|
            @added_trip.add_gift( 1, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "adding to middle (2)" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
          @added_trip = StolenSleigh::Trip.new(0)
          @added_trip.add_gift( 0, 69913 )
          @added_trip.add_gift( 1,  21069 )

          [ 31517, 51457, 36237, 74932, 58021, 93699 ].each_with_index do |gift_id, idx|
            @added_trip.add_gift( idx+1, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "with cloning (1)" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
          @added_trip = StolenSleigh::Trip.new(0)
          @added_trip.add_gift( 0, 69913 )
          @added_trip.add_gift( 1,  21069 )

          [ 31517, 51457, 36237, 74932, 58021, 93699 ].each_with_index do |gift_id, idx|
            @added_trip = @added_trip.clone
            @added_trip.add_gift( idx+1, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "with cloning (2)" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
          @added_trip = StolenSleigh::Trip.new(0)
          @added_trip.add_gift( 0, 69913 )
          @added_trip.add_gift( 1,  21069 )

          [ 31517, 51457, 36237, 74932, 58021, 93699 ].reverse.each do |gift_id, idx|
            @added_trip = @added_trip.clone
            @added_trip.add_gift( 1, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end

      context "with cloning (3)" do
        before :each do
          @created_trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069] )
          @added_trip = StolenSleigh::Trip.new(0)
          [ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069].reverse.each do |gift_id|
            @added_trip = @added_trip.clone
            @added_trip.add_gift( 0, gift_id )
          end
        end

        it "has gifts in expected order" do
          expect( @added_trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069]
        end

        it "has same weight" do
          expect( @added_trip.gifts_weight ).to be_within(1e-5).of @created_trip.gifts_weight
        end

        it "has same score" do
          expect( @added_trip.score ).to be_within(1e-5).of @created_trip.score
        end
      end
    end

    describe "#lowest_cost_insert_at" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ] )
      end

      it "finds best insert point for given id" do
        pos, cost = @trip.lowest_cost_insert_at( 31517 )
        expect( pos ).to eql 1
      end

      it "returns cost" do
        pos, cost = @trip.lowest_cost_insert_at( 31517 )
        expect( cost ).to be_within(1e-6).of 30504.9449995
      end

      it "returns cost same as insertion difference" do
        before_score = @trip.score
        pos, cost = @trip.lowest_cost_insert_at( 31517 )
        @trip.add_gift( pos, 31517 )
        after_score = @trip.score
        expect( cost ).to be_within(1e-6).of ( after_score - before_score )
      end

      it "works for several known positions" do
        test_data = [
          Hash[ :gifts => [ 31517, 51457, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 69913, :expected_insert_at => 0, :expected_cost =>  28668.292 ],
          Hash[ :gifts => [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 31517, :expected_insert_at => 1, :expected_cost =>  30504.945 ],
          Hash[ :gifts => [ 69913, 31517, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 51457, :expected_insert_at => 2, :expected_cost =>  47182.820 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 74932, 58021, 93699, 21069 ], :gift_id => 36237, :expected_insert_at => 3, :expected_cost => 101624.980 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 58021, 93699, 21069 ], :gift_id => 74932, :expected_insert_at => 4, :expected_cost =>   2666.185 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 93699, 21069 ], :gift_id => 58021, :expected_insert_at => 5, :expected_cost =>  48196.495 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 58021, 21069 ], :gift_id => 93699, :expected_insert_at => 6, :expected_cost =>   2950.885 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 58021, 93699 ], :gift_id => 21069, :expected_insert_at => 7, :expected_cost =>  41598.151 ]
        ]

        test_data.each do |test_item|
          trip = StolenSleigh::Trip.from_narray( NArray[ *test_item[:gifts] ] )
          pos, cost = trip.lowest_cost_insert_at( test_item[:gift_id] )
          expect( pos ).to eql test_item[:expected_insert_at]
          expect( cost ).to be_within(1e-3).of  test_item[:expected_cost]
        end
      end

      it "works for cloned trips" do
        test_data = [
          Hash[ :gifts => [ 31517, 51457, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 69913, :expected_insert_at => 0, :expected_cost =>  28668.292 ],
          Hash[ :gifts => [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 31517, :expected_insert_at => 1, :expected_cost =>  30504.945 ],
          Hash[ :gifts => [ 69913, 31517, 36237, 74932, 58021, 93699, 21069 ], :gift_id => 51457, :expected_insert_at => 2, :expected_cost =>  47182.820 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 74932, 58021, 93699, 21069 ], :gift_id => 36237, :expected_insert_at => 3, :expected_cost => 101624.980 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 58021, 93699, 21069 ], :gift_id => 74932, :expected_insert_at => 4, :expected_cost =>   2666.185 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 93699, 21069 ], :gift_id => 58021, :expected_insert_at => 5, :expected_cost =>  48196.495 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 58021, 21069 ], :gift_id => 93699, :expected_insert_at => 6, :expected_cost =>   2950.885 ],
          Hash[ :gifts => [ 69913, 31517, 51457, 36237, 74932, 58021, 93699 ], :gift_id => 21069, :expected_insert_at => 7, :expected_cost =>  41598.151 ]
        ]

        test_data.each do |test_item|
          trip = StolenSleigh::Trip.from_narray( NArray[ *test_item[:gifts] ] ).clone
          pos, cost = trip.lowest_cost_insert_at( test_item[:gift_id] )
          expect( pos ).to eql test_item[:expected_insert_at]
          expect( cost ).to be_within(1e-3).of  test_item[:expected_cost]
        end
      end

      it "works for cloned trips, after adding" do
        test_data = [
          Hash[ :gifts => [ 51457, 36237, 74932, 58021, 93699, 21069 ], :add_gift_id => 31517, :gift_id => 69913, :expected_insert_at => 0, :expected_cost =>  28668.292 ],
          Hash[ :gifts => [ 51457, 36237, 74932, 58021, 93699, 21069 ], :add_gift_id => 69913, :gift_id => 31517, :expected_insert_at => 1, :expected_cost =>  30504.945 ],
          Hash[ :gifts => [ 31517, 36237, 74932, 58021, 93699, 21069 ], :add_gift_id => 69913, :gift_id => 51457, :expected_insert_at => 2, :expected_cost =>  47182.820 ],
          Hash[ :gifts => [ 31517, 51457, 74932, 58021, 93699, 21069 ], :add_gift_id => 69913, :gift_id => 36237, :expected_insert_at => 3, :expected_cost => 101624.980 ],
          Hash[ :gifts => [ 31517, 51457, 36237, 58021, 93699, 21069 ], :add_gift_id => 69913, :gift_id => 74932, :expected_insert_at => 4, :expected_cost =>   2666.185 ],
          Hash[ :gifts => [ 31517, 51457, 36237, 74932, 93699, 21069 ], :add_gift_id => 69913, :gift_id => 58021, :expected_insert_at => 5, :expected_cost =>  48196.495 ],
          Hash[ :gifts => [ 31517, 51457, 36237, 74932, 58021, 21069 ], :add_gift_id => 69913, :gift_id => 93699, :expected_insert_at => 6, :expected_cost =>   2950.885 ],
          Hash[ :gifts => [ 31517, 51457, 36237, 74932, 58021, 93699 ], :add_gift_id => 69913, :gift_id => 21069, :expected_insert_at => 7, :expected_cost =>  41598.151 ]
        ]

        test_data.each do |test_item|
          trip = StolenSleigh::Trip.from_narray( NArray[ *test_item[:gifts] ] ).clone
          trip.add_gift( 0, test_item[:add_gift_id ] )
          pos, cost = trip.lowest_cost_insert_at( test_item[:gift_id] )
          expect( pos ).to eql test_item[:expected_insert_at]
          expect( cost ).to be_within(1e-3).of  test_item[:expected_cost]
        end
      end
    end

    describe "#remove_gifts_by_idx" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
      end

      it "removes specific gift" do
        @trip.remove_gifts_by_idx( NArray[1] )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
      end

      it "removes specific gifts" do
        @trip.remove_gifts_by_idx( NArray[3,5] )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 74932, 93699, 21069 ]
      end

      it "works alongside other changes, without anything breaking" do
        before_score = @trip.score
        @trip.remove_gifts_by_idx( NArray[3,5] )
        # Add back plus a clone in-between
        @trip.add_gift( @trip.lowest_cost_insert_at( 36237 ).first, 36237 )
        @trip = @trip.clone
        @trip.add_gift( @trip.lowest_cost_insert_at( 58021 ).first, 58021 )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        expect( @trip.score ).to be_within( 1e-6 ).of before_score
      end
    end

    describe "#remove_gifts_by_gift_id" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
      end

      it "removes specific gift" do
        @trip.remove_gifts_by_gift_id( NArray[31517] )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
      end

      it "removes specific gifts" do
        @trip.remove_gifts_by_gift_id( NArray[31517,58021] )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 51457, 36237, 74932, 93699, 21069 ]
      end

      it "works alongside other changes, without anything breaking" do
        before_score = @trip.score
        @trip.remove_gifts_by_gift_id( NArray[31517,58021] )
        # Add back plus a clone in-between
        @trip.add_gift( @trip.lowest_cost_insert_at( 31517 ).first, 31517 )
        @trip = @trip.clone
        @trip.add_gift( @trip.lowest_cost_insert_at( 58021 ).first, 58021 )
        expect( @trip.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        expect( @trip.score ).to be_within( 1e-6 ).of before_score
      end
    end

    describe "#sample_gifts" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
        @trip_remaining = StolenSleigh::Trip.new(0)
        StolenSleigh.srand_array( [20,30,40,50] )
      end

      it "can sample a single item" do
        results = @trip.sample_gifts( 1, @trip_remaining )
        expect( results ).to be_narray_like NArray[ 31517 ]
        expect( @trip_remaining.gifts ).to be_narray_like NArray[ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
      end

      it "can sample two items" do
        results = @trip.sample_gifts( 2, @trip_remaining )
        expect( results ).to be_narray_like NArray[ 31517, 74932 ]
        expect( @trip_remaining.gifts ).to be_narray_like NArray[ 69913, 51457, 36237, 58021, 93699, 21069 ]
      end

      it "copies to remaining trip without corruption" do
        results = @trip.sample_gifts( 2, @trip_remaining )
        @trip_remaining.add_gift( @trip_remaining.lowest_cost_insert_at( 31517 ).first, 31517 )
        @trip_remaining.add_gift( @trip_remaining.lowest_cost_insert_at( 74932 ).first, 74932 )
        expect( @trip_remaining.score ).to be_within(1e-6).of @trip.score
      end

      it "can oversample down to nothing remaining" do
        results = @trip.sample_gifts( 100, @trip_remaining )
        expect( results ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ]
        expect( @trip_remaining.gifts ).to be_empty
        expect( @trip_remaining.score ).to eql 0.0
      end
    end

    describe "#sample_gifts_var" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 31517, 51457, 36237, 74932, 58021, 93699, 21069 ] )
        @trip_remaining = StolenSleigh::Trip.new(0)
        StolenSleigh.srand_array( [20,30,40,50] )
      end

      it "can sample a single item" do
        results = @trip.sample_gifts_var( 1, @trip_remaining )
        expect( results ).to be_narray_like NArray[ 74932 ]
        expect( @trip_remaining.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 58021, 93699, 21069 ]
      end

      it "can sample less than requested items" do
        results = @trip.sample_gifts_var( 4, @trip_remaining )
        expect( results ).to be_narray_like NArray[ 74932, 58021 ]
        expect( @trip_remaining.gifts ).to be_narray_like NArray[ 69913, 31517, 51457, 36237, 93699, 21069 ]
      end

      it "copies to remaining trip without corruption" do
        results = @trip.sample_gifts_var( 4, @trip_remaining )
        @trip_remaining.add_gift( @trip_remaining.lowest_cost_insert_at( 74932 ).first, 74932 )
        @trip_remaining.add_gift( @trip_remaining.lowest_cost_insert_at( 58021 ).first, 58021 )
        expect( @trip_remaining.score ).to be_within(1e-6).of @trip.score
      end
    end

    describe "#lowest_cost_break_at" do
      before :each do
        @trip = StolenSleigh::Trip.from_narray( NArray[ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ] )
      end

      it "returns position 0, cost 0.0 for unbreakable trip" do
        pos, cost = @trip.lowest_cost_break_at
        expect( pos ).to eql 0
        expect( cost ).to eql 0.0
      end

      it "suggests correct split point for an obviously-splittable trip" do
        @trip = StolenSleigh::Trip.from_narray( NArray[20153, 50941, 36192, 66800, 410, 52313, 63464, # Split here, before pos 7
                                                       98506, 21476, 50355, 9619, 5997, 90790 ] )
        pos, cost = @trip.lowest_cost_break_at
        expect( pos ).to eql 7
        expect( cost ).to be < 0.0
      end
    end

    describe "#splice" do
      before :each do
        @test_gifts = [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
        @trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
        @before_num_gifts = @trip.num_gifts
        @before_score = @trip.score
        @before_weight = @trip.gifts_weight

        @out_trip = StolenSleigh::Trip.new(0)
      end

      it "does nothing when input trip is empty and there is nothing to splice out" do
        in_trip = StolenSleigh::Trip.new(0)
        (0..7).each do |idx|
          @trip.splice( idx, 0, in_trip, @out_trip )
          expect( @trip.gifts_weight ).to be_within(1e-6).of @before_weight
          expect( @trip.score ).to be_within(1e-6).of @before_score
          expect( @out_trip.num_gifts ).to eql 0
        end
      end

      (0..6).each do |idx|
        it "splices out a single gift at position #{idx}" do
          in_trip = StolenSleigh::Trip.new(0)
          expected_gift_id = @test_gifts[idx]
          @test_gifts = @test_gifts - [expected_gift_id]
          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ expected_gift_id ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 1, in_trip, @out_trip )
          expect( @trip.gifts_weight ).to be < @before_weight
          expect( @trip.score ).to be < @before_score
          expect( @trip.num_gifts ).to eql 6
          expect( @out_trip.num_gifts ).to eql 1

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end

        it "splices out a single gift at position #{idx} and replaces with a new gift" do
          in_trip = StolenSleigh::Trip.from_narray( NArray[20000] )
          expected_gift_id = @test_gifts[idx]
          @test_gifts.map! { |gift_id| gift_id == expected_gift_id ? 20000 : gift_id }
          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ expected_gift_id ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 1, in_trip, @out_trip )

          expect( @trip.num_gifts ).to eql 7
          expect( @out_trip.num_gifts ).to eql 1

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end

        it "splices out a zero gifts at position #{idx} and inserts two new gifts" do
          in_trip = StolenSleigh::Trip.from_narray( NArray[20000, 19000] )

          expected_gift_id = @test_gifts[idx]
          if idx == 0
            @test_gifts = [20000, 19000] + @test_gifts
          else
            @test_gifts = @test_gifts[0..idx-1] +  [20000, 19000] + @test_gifts[idx..6]
          end

          expected_out_trip = StolenSleigh::Trip.new(0)
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 0, in_trip, @out_trip )

          expect( @trip.num_gifts ).to eql 9
          expect( @out_trip.num_gifts ).to eql 0

          expect( @out_trip.gifts ).to be_empty
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end
      end

      (0..5).each do |idx|
        it "splices out two gifts at position #{idx}" do
          in_trip = StolenSleigh::Trip.new(0)
          expected_gift_ids = [ @test_gifts[idx], @test_gifts[idx+1] ]
          @test_gifts = @test_gifts - expected_gift_ids
          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 2, in_trip, @out_trip )
          expect( @trip.gifts_weight ).to be < @before_weight
          expect( @trip.score ).to be < @before_score
          expect( @trip.num_gifts ).to eql 5
          expect( @out_trip.num_gifts ).to eql 2

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end

        it "splices out two gifts at position #{idx} and replaces with single new gift" do
          in_trip = StolenSleigh::Trip.from_narray( NArray[20000] )
          expected_gift_id1 = @test_gifts[idx]
          @test_gifts.map! { |gift_id| gift_id == expected_gift_id1 ? 20000 : gift_id }
          expected_gift_id2 = @test_gifts[idx+1]
          @test_gifts.reject! { |gift_id| gift_id == expected_gift_id2 }

          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ expected_gift_id1, expected_gift_id2 ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 2, in_trip, @out_trip )

          expect( @trip.num_gifts ).to eql 6
          expect( @out_trip.num_gifts ).to eql 2

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end

        it "splices out two gifts at position #{idx} and replaces with two new gifts" do
          in_trip = StolenSleigh::Trip.from_narray( NArray[20000, 19000] )
          expected_gift_id1 = @test_gifts[idx]
          @test_gifts.map! { |gift_id| gift_id == expected_gift_id1 ? 20000 : gift_id }
          expected_gift_id2 = @test_gifts[idx+1]
          @test_gifts.map! { |gift_id| gift_id == expected_gift_id2 ? 19000 : gift_id }

          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ expected_gift_id1, expected_gift_id2 ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 2, in_trip, @out_trip )

          expect( @trip.num_gifts ).to eql 7
          expect( @out_trip.num_gifts ).to eql 2

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end
      end

      (0..4).each do |idx|
        it "splices out three gifts at position #{idx}" do
          in_trip = StolenSleigh::Trip.new(0)
          expected_gift_ids = [ @test_gifts[idx], @test_gifts[idx+1], @test_gifts[idx+2] ]

          @test_gifts = @test_gifts - expected_gift_ids
          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 3, in_trip, @out_trip )
          expect( @trip.gifts_weight ).to be < @before_weight
          expect( @trip.score ).to be < @before_score
          expect( @trip.num_gifts ).to eql 4
          expect( @out_trip.num_gifts ).to eql 3

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end

        it "splices out three gifts at position #{idx} and replaces with 2 other gifts" do
          in_trip = StolenSleigh::Trip.from_narray( NArray[20000, 19000] )
          expected_gift_ids = [ @test_gifts[idx], @test_gifts[idx+1], @test_gifts[idx+2] ]
          @test_gifts.map! { |gift_id| gift_id == expected_gift_ids[0] ? 20000 : gift_id }
          @test_gifts.map! { |gift_id| gift_id == expected_gift_ids[1] ? 19000 : gift_id }
          @test_gifts = @test_gifts - expected_gift_ids

          expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
          expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
          @trip.splice( idx, 3, in_trip, @out_trip )
          expect( @trip.num_gifts ).to eql 6
          expect( @out_trip.num_gifts ).to eql 3

          expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
          expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

          expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
          expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

          expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
          expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
        end
      end

      it "caps length of splice out to 0 at end" do
        in_trip = StolenSleigh::Trip.new(0)
        expected_out_trip = StolenSleigh::Trip.new(0)
        expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
        @trip.splice( 7, 1, in_trip, @out_trip )
        expect( @trip.gifts_weight ).to eql @before_weight
        expect( @trip.score ).to be_within(1e-6).of @before_score
        expect( @trip.num_gifts ).to eql 7
        expect( @out_trip.num_gifts ).to eql 0

        expect( @out_trip.gifts ).to be_empty
        expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

        expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
        expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

        expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
        expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
      end

      it "caps length of splice out to 0 at end and inserts at end" do
        in_trip = StolenSleigh::Trip.from_narray( NArray[20000, 19000] )
        expected_out_trip = StolenSleigh::Trip.new(0)
        expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *(@test_gifts +[20000, 19000]) ] )
        @trip.splice( 7, 1, in_trip, @out_trip )
        expect( @trip.num_gifts ).to eql 9
        expect( @out_trip.num_gifts ).to eql 0

        expect( @out_trip.gifts ).to be_empty
        expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

        expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
        expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

        expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
        expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
      end

      it "caps length of splice out to 2 close to end (for a 3 splice)" do
        in_trip = StolenSleigh::Trip.new(0)
        expected_gift_ids = [ @test_gifts[5], @test_gifts[6] ]
        @test_gifts = @test_gifts - expected_gift_ids
        expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
        expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
        @trip.splice( 5, 3, in_trip, @out_trip )
        expect( @trip.gifts_weight ).to be < @before_weight
        expect( @trip.score ).to be < @before_score
        expect( @trip.num_gifts ).to eql 5
        expect( @out_trip.num_gifts ).to eql 2

        expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
        expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

        expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
        expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

        expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
        expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
      end

      it "caps length of splice out to 1 close to end (for a 3 splice)" do
        in_trip = StolenSleigh::Trip.new(0)
        expected_gift_ids = [ @test_gifts[6] ]
        @test_gifts = @test_gifts - expected_gift_ids
        expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
        expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
        @trip.splice( 6, 3, in_trip, @out_trip )
        expect( @trip.gifts_weight ).to be < @before_weight
        expect( @trip.score ).to be < @before_score
        expect( @trip.num_gifts ).to eql 6
        expect( @out_trip.num_gifts ).to eql 1

        expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
        expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

        expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
        expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

        expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
        expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
      end

      it "caps length of splice out to 1 close to end (for a 2 splice)" do
        in_trip = StolenSleigh::Trip.new(0)
        expected_gift_ids = [ @test_gifts[6] ]
        @test_gifts = @test_gifts - expected_gift_ids
        expected_out_trip = StolenSleigh::Trip.from_narray( NArray[ *expected_gift_ids ] )
        expected_remaining_trip = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts ] )
        @trip.splice( 6, 2, in_trip, @out_trip )
        expect( @trip.gifts_weight ).to be < @before_weight
        expect( @trip.score ).to be < @before_score
        expect( @trip.num_gifts ).to eql 6
        expect( @out_trip.num_gifts ).to eql 1

        expect( @out_trip.gifts ).to be_narray_like expected_out_trip.gifts
        expect( @trip.gifts ).to be_narray_like expected_remaining_trip.gifts

        expect( @out_trip.gifts_weight ).to be_within(1e-6).of expected_out_trip.gifts_weight
        expect( @trip.gifts_weight ).to be_within(1e-6).of expected_remaining_trip.gifts_weight

        expect( @out_trip.score ).to be_within(1e-6).of expected_out_trip.score
        expect( @trip.score ).to be_within(1e-6).of expected_remaining_trip.score
      end
    end

    describe "#cross_splice and #cross_splice_score" do
      before :each do
        @test_gifts_a = [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
        @test_gifts_b = [ 69915, 51459, 36239, 74934, 58024, 93702, 21071 ]

        @trip_a = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts_a ] )
        @trip_b = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts_b ] )

        @before_score = @trip_a.score + @trip_b.score
      end

      it "make expected changes to gifts array" do
        @trip_a.cross_splice( 4, 3, 3, @trip_b, 4, 3, 3 )
        expect_gifts_a = NArray[ 69913, 51457, 36237, 58024, 93702, 21071, 74932 ]
        expect_gifts_b = NArray[ 69915, 51459, 36239, 58021, 93699, 21069, 74934 ]
        expect( @trip_a.gifts ).to be_narray_like expect_gifts_a
        expect( @trip_b.gifts ).to be_narray_like expect_gifts_b
      end

      it "agree on eventual score" do
        expect( @trip_a.cross_splice_score( 4, 3, 3, @trip_b, 4, 3, 3 ) ).to be_within(1e-6).of 3329719.304570
      end
    end

    describe "#splice_from_a_to_b" do
      before :each do
        @test_gifts_a = [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]
        @test_gifts_b = [ 69915, 51459, 36239, 74934, 58024, 93702, 21071 ]

        @trip_a = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts_a ] )
        @trip_b = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts_b ] )

        @before_score = @trip_a.score + @trip_b.score
      end

      it "makes expected changes to gifts array" do
        @trip_a.splice_from_a_to_b( 4, 3, @trip_b, 2 )
        expect_gifts_a = NArray[ 69913, 51457, 36237, 74932 ]
        expect_gifts_b = NArray[ 69915, 51459, 58021, 93699, 21069, 36239, 74934, 58024, 93702, 21071 ]
        expect( @trip_a.gifts ).to be_narray_like expect_gifts_a
        expect( @trip_b.gifts ).to be_narray_like expect_gifts_b
      end

      it "scores same as manually-constructed trips with same gifts" do
        @trip_a.splice_from_a_to_b( 4, 3, @trip_b, 2 )
        expect_gifts_a = NArray[ 69913, 51457, 36237, 74932 ]
        expect_gifts_b = NArray[ 69915, 51459, 58021, 93699, 21069, 36239, 74934, 58024, 93702, 21071 ]

        expect( @trip_a.score ).to be_within(1e-6).of StolenSleigh::Trip.from_narray( expect_gifts_a ).score
        expect( @trip_b.score ).to be_within(1e-6).of StolenSleigh::Trip.from_narray( expect_gifts_b ).score
      end

      it "scores same as predicted by splice_from_a_to_b_score" do
        predicted_score = @trip_a.splice_from_a_to_b_score( 4, 3, @trip_b, 2 )
        @trip_a.splice_from_a_to_b( 4, 3, @trip_b, 2 )
        expect( @trip_a.score + @trip_b.score ).to be_within(1e-6).of predicted_score
      end

      it "can empty the source trip" do
        @trip_a.splice_from_a_to_b( 0, 7, @trip_b, 2 )
        expect_gifts_b = NArray[ 69915, 51459, 69913, 51457, 36237, 74932, 58021, 93699, 21069, 36239, 74934, 58024, 93702, 21071 ]
        expect( @trip_a.gifts ).to be_empty
        expect( @trip_b.gifts ).to be_narray_like expect_gifts_b
      end

      it "scores correctly when emptying the source trip" do
        predicted_score = @trip_a.splice_from_a_to_b_score( 0, 7, @trip_b, 2 )
        @trip_a.splice_from_a_to_b( 0, 7, @trip_b, 2 )
        expect( @trip_a.score + @trip_b.score ).to be_within(1e-6).of predicted_score
        expect( @trip_a.score ).to eql 0.0
      end

      it "can splice to the end of the destination trip" do
        @trip_a.splice_from_a_to_b( 4, 3, @trip_b, 7 )
        expect_gifts_a = NArray[ 69913, 51457, 36237, 74932 ]
        expect_gifts_b = NArray[ 69915, 51459, 36239, 74934, 58024, 93702, 21071, 58021, 93699, 21069 ]
        expect( @trip_a.gifts ).to be_narray_like expect_gifts_a
        expect( @trip_b.gifts ).to be_narray_like expect_gifts_b
      end

      it "scores correctly when splicing to the end of the destination trip" do
        predicted_score = @trip_a.splice_from_a_to_b_score(  4, 3, @trip_b, 7 )
        @trip_a.splice_from_a_to_b( 4, 3, @trip_b, 7 )
        expect( @trip_a.score + @trip_b.score ).to be_within(1e-6).of predicted_score
      end
    end

    describe "#splice_self" do
      before :each do
        @test_gifts_a = [ 69913, 51457, 36237, 74932, 58021, 93699, 21069 ]

        @trip_a = StolenSleigh::Trip.from_narray( NArray[ *@test_gifts_a ] )

        @before_score = @trip_a.score
      end

      it "makes expected changes to gifts array" do
        @trip_a.splice_self( 4, 3, 2 )
        expect_gifts_a = NArray[ 69913, 51457, 58021, 93699, 21069, 36237, 74932 ]
        expect( @trip_a.gifts ).to be_narray_like expect_gifts_a
      end

      it "scores same as manually-constructed trips with same gifts" do
        @trip_a.splice_self( 4, 3, 2 )
        expect_gifts_a = NArray[ 69913, 51457, 58021, 93699, 21069, 36237, 74932 ]

        expect( @trip_a.score ).to be_within(1e-6).of StolenSleigh::Trip.from_narray( expect_gifts_a ).score
      end

      it "scores same as predicted by splice_self_score" do
        predicted_score = @trip_a.splice_self_score( 4, 3, 2 )
        @trip_a.splice_self( 4, 3, 2 )
        expect( @trip_a.score ).to be_within(1e-6).of predicted_score
      end
    end

  end
end
