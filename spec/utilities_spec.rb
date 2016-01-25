require 'helpers'

describe StolenSleigh do
  describe "haversine distance" do
    it "returns expected distance between BNA and LAX" do
     m = Math::PI / 180
     lat_bna = m * 36.12
     lng_bna = m * -86.67
     lat_lax = m * 33.94
     lng_lax = m * -118.40
     expect( StolenSleigh.haversine_distance( lat_lax, lng_lax, lat_bna, lng_bna ) ).to be_within( 1e-3 ).of 2886.444
     expect( StolenSleigh.haversine_distance( lat_bna, lng_bna, lat_lax, lng_lax ) ).to be_within( 1e-3 ).of 2886.444
    end
  end

  describe "distance between gifts" do
    before :each do
      @input = StolenSleigh.input
    end

    it "returns haversine distance between two chosen gifts" do
      [1,2,3,4,7,20,7,100,300,9000,23123,99999,23123].each_cons(2) do |gift_a_id, gift_b_id|
        lat_a = @input.gifts[0,gift_a_id]
        lng_a = @input.gifts[1,gift_a_id]

        lat_b = @input.gifts[0,gift_b_id]
        lng_b = @input.gifts[1,gift_b_id]

        distance_a_b = StolenSleigh.haversine_distance( lat_a, lng_a, lat_b, lng_b )

        expect( StolenSleigh.gift_distance( gift_a_id, gift_b_id ) ).to be_within( 1e-6 ).of distance_a_b
      end
    end
  end
end
