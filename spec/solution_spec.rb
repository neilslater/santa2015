require 'helpers'

describe StolenSleigh::Solution do
  describe "class methods" do
    describe "#new" do
      it "creates a new object" do
        expect( StolenSleigh::Solution.new ).to be_a StolenSleigh::Solution
      end
    end

    describe "with Marshal" do
      before do
        @orig_solution = StolenSleigh::Solution.new
        (0..100).each do |i|
          @orig_solution.items[0,i] = i
          @orig_solution.items[1,i] = i % 5
        end
        @saved_data = Marshal.dump( @orig_solution )
        @copy_solution =  Marshal.load( @saved_data )
      end

      it "can save and retrieve training data" do
        expect( @copy_solution ).to_not be @orig_solution
        expect( @copy_solution.items ).to be_an NArray
        orig_items = @orig_solution.items
        copy_items = @copy_solution.items
        expect( copy_items ).to_not be orig_items
        expect( copy_items ).to be_narray_like orig_items
      end
    end
  end

  describe "instance methods" do
    def make_valid_solution sol
      (0..99_999).each do |i|
        sol.items[0,i] = i
        sol.items[1,i] = i
      end
    end

    before :each do
      @solution = StolenSleigh::Solution.new
      (0..200).each do |i|
        @solution.items[0,i] = i + 7
        @solution.items[1,i] = i % 20
      end
    end

    describe "#clone" do
      it "makes deep copy of solution data" do
        @copy_solution = @solution.clone
        expect( @copy_solution ).to_not be @solution
        expect( @copy_solution.items ).to be_an NArray
        orig_items = @solution.items
        copy_items = @copy_solution.items
        expect( copy_items ).to_not be orig_items
        expect( copy_items ).to be_narray_like orig_items
      end
    end

    describe "#validate" do
      it "fails when not all gift_ids are set" do
        expect {
          @solution.validate
        }.to raise_error RuntimeError, /Not all gifts are in the solution/
      end

      it "fails when a single gift_id is repeated" do
        make_valid_solution( @solution )
        @solution.items[0,87654] = 12
        expect {
          @solution.validate
        }.to raise_error RuntimeError, /Not all gifts are in the solution/
      end

      it "fails when the weight is too high on a single trip" do
        make_valid_solution( @solution )
        trip_id = 789
        (789..900).each do |idx|
          @solution.items[1,idx] = trip_id
        end
        expect {
          @solution.validate
        }.to raise_error RuntimeError, /Non-valid trip found/
      end

      it "passes for a basic solution of one trip per item" do
        make_valid_solution( @solution )
        expect( @solution.validate ).to be true
      end

      it "passes for the sample solution" do
        @solution.import_csv( File.join(StolenSleigh::DATA_PATH, 'sample_submission.csv' ) )
        expect( @solution.validate ).to be true
      end
    end

    describe "#generate_trip" do
      it "exports trip data for identified trip" do
        @solution.import_csv( File.join(StolenSleigh::DATA_PATH, 'sample_submission.csv' ) )
        trip = @solution.generate_trip( 75 )
        expect( trip.num_gifts ).to be 20
        expect( trip.gifts ).to be_narray_like( NArray[ 75, 5075, 10075, 15075, 20075, 25075, 30075,
            35075, 40075, 45075, 50075, 55075, 60075, 65075, 70075, 75075, 80075, 85075, 90075, 95075] )
      end
    end

    describe "#score" do
      it "is correct for a basic solution of one trip per item" do
        make_valid_solution( @solution )
        expect( @solution.score ).to be_within(0.1).of 29121011015.6
      end
    end
  end
end
