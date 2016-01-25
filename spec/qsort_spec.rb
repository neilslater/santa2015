require 'helpers'

describe StolenSleigh do
  describe "internal quicksort" do
    it "sorts integer ids by float properties" do
      ids = ids = NArray[0,4,2,3,1]
      props = NArray[0.1,0.2,0.3,0.4,0.5]
      expect( StolenSleigh.sort_ids_by_float( ids, props ) ).to be_narray_like NArray[0,1,2,3,4]
      expect( ids ).to be_narray_like NArray[0,1,2,3,4]
    end
  end
end
