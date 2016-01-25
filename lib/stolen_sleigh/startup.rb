require 'csv'

module StolenSleigh
  DATA_PATH = File.realpath( File.join( File.dirname(__FILE__), '../..', 'data' ) )
  OUTPUT_PATH = File.realpath( File.join( File.dirname(__FILE__), '../..', 'output' ) )

  CSV_PATH = File.join( DATA_PATH, 'gifts.csv' )
  IMPORTED_PATH = File.join( DATA_PATH, 'input.dat' )

  if File.exists?( IMPORTED_PATH )
    imported_data = StolenSleigh::Input.load( IMPORTED_PATH )
  else
    imported_data = nil
    puts "Cannot load data, need to run first-time import."
  end

  INPUT = imported_data
  if (INPUT)
    self.init_gifts
  end

  def self.input
    INPUT
  end

  def self.ready_to_import?
    File.exists?( CSV_PATH )
  end

  def self.import_from_csv
    input = Input.new

    data = input.gifts

    puts "Reading #{CSV_PATH}"

    csv = CSV.open( CSV_PATH )
    header = csv.readline
    unless header == ["GiftId","Latitude","Longitude","Weight"]
      raise "Did not recognise header #{header.inspect}"
    end

    np_lat = Math::PI/2
    np_long = 0.0

    csv.each do |line|
      gift_id  = line[0].to_i
      lat  = line[1].to_f * Math::PI/180
      long = line[2].to_f * Math::PI/180
      weight = line[3].to_f

      np_distance = haversine_distance( np_lat, np_long, lat, long )
      # Cost for solo journey (including weight of sleigh)
      base_cost = ( 20.0 + weight ) * np_distance

      gift_x = 0.5 * Math.cos(lat) * Math.sin(long)
      gift_y = 0.5 * Math.cos(lat) * Math.cos(long)
      gift_z = 0.5 * Math.sin(lat)

      data[0, gift_id-1] = lat
      data[1, gift_id-1] = long
      data[2, gift_id-1] = weight
      data[3, gift_id-1] = np_distance
      data[4, gift_id-1] = base_cost
      data[5, gift_id-1] = gift_x
      data[6, gift_id-1] = gift_y
      data[7, gift_id-1] = gift_z
    end

    input.save( IMPORTED_PATH )

    # Removing the constant first avoids warnings
    StolenSleigh.send( :remove_const, :INPUT )
    StolenSleigh.send( :const_set, :INPUT, input )
    self.init_gifts
    input
  end

  def self.ready_to_run?
    !! ( INPUT )
  end
end
