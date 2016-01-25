require 'helpers'

describe StolenSleigh::Optimiser::Shaker do
  describe "#test_multi_draw" do
    it "takes samples from trips and returns them" do
      StolenSleigh.srand( 76 )
      remaining, sampled = StolenSleigh::Optimiser::Shaker.test_multi_draw(
        [ StolenSleigh::Trip.from_narray( NArray[1,2,3,4] ),
          StolenSleigh::Trip.from_narray( NArray[5,6,7,8] ),
          StolenSleigh::Trip.from_narray( NArray[9,10] ) ], 3 )

      remaining.each do |remaining_trip|
        expect( remaining_trip ).to be_a StolenSleigh::Trip
      end

      expect( remaining[0].gifts ).to be_narray_like NArray[2,3]
      expect( remaining[1].gifts ).to be_narray_like NArray[5,7,8]
      expect( remaining[2].gifts ).to be_narray_like NArray[9]

      expect( sampled ).to be_narray_like NArray[ 1, 4, 6, 10 ]
    end
  end
end
