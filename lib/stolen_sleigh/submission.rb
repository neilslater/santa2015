require 'csv'

class StolenSleigh::Solution
  CSV_HEADERS = %w(GiftId TripId)

  def write_csv csv_filename
    csv = CSV.open( csv_filename, 'wb' )
    csv << CSV_HEADERS

    (0..99_999).each do |row_id|
      row = [
        items[0,row_id] + 1,
        items[1,row_id]
      ]
      csv << row
    end
    csv.close
  end

  def import_csv csv_filename
    csv = CSV.open( csv_filename, 'r' )

    headers = csv.readline
    if headers.first == 'GiftId' && headers.last == 'TripId' && headers.count == 2
      gift_col = 0
      trip_col = 1
    elsif headers.first == 'TripId' && headers.last == 'GiftId'
      gift_col = 1
      trip_col = 0
    else
      raise "Bad headers: #{headers.inspect}" && headers.count == 2
    end

    (0..99_999).each do |row_id|
      row = csv.readline.map {|s| s.to_i}
      items[0,row_id] = row[gift_col]-1
      items[1,row_id] = row[trip_col]
    end
    csv.close
  end
end
