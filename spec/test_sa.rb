require 'stolen_sleigh'
s = StolenSleigh::Solution.new
s.import_csv('output/submission_btb_12426908796.csv')
tc = StolenSleigh::TripCollection.from_solution( s )
trip_scores = Hash.new
(0..1532).each { |trip_id| if trip = tc.get_trip(trip_id) then trip_scores[ trip_id ] = trip.score; end }
trip_scores.sort_by {|k,v| v }.last
trip = tc.get_trip( 1506 )
gifts = trip.gifts
t = 2500
f = 0.99
while ( t > 40 ) do  p StolenSleigh.anneal( gifts, t, 100000 ); t *= f; end
