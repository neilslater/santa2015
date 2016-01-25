require "narray"

# StolenSleigh adds support for Marshal to NArray.
# Code originally from http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-talk/194510
class NArray
  def _dump *ignored
    Marshal.dump :typecode => typecode, :shape => shape, :data => to_s
  end
  def self._load buf
    h = Marshal.load buf
    typecode = h[:typecode]
    shape = h[:shape]
    data = h[:data]
    to_na data, typecode, *shape
  end
end

class StolenSleigh::Input
  # @!visibility private
  # Adds support for Marshal, via to_h and from_h methods
  def to_h
    Hash[
      :gifts => self.gifts
    ]
  end

  # @!visibility private
  # Constructs a StolenSleigh::Input from hash description. Used internally to support Marshal.
  # @param [Hash] h Keys are :gifts
  # @return [StolenSleigh::Input] new object
  def self.from_h h
    StolenSleigh::Input.from_narray( h[:gifts] )
  end

  # @!visibility private
  def _dump *ignored
    Marshal.dump to_h
  end

  # @!visibility private
  def self._load buf
    h = Marshal.load buf
    from_h h
  end

  def save filename
    File.open( filename, 'wb' ) { |file| Marshal.dump( self, file ) }
  end

  def self.load filename
    File.open( filename, 'rb' ) { |file| Marshal.load(file) }
  end
end


class StolenSleigh::Solution
  # @!visibility private
  # Adds support for Marshal, via to_h and from_h methods
  def to_h
    Hash[
      :items => self.items
    ]
  end

  # @!visibility private
  # Constructs a StolenSleigh::Solution from hash description. Used internally to support Marshal.
  # @param [Hash] h Keys are :items
  # @return [StolenSleigh::Solution] new object
  def self.from_h h
    StolenSleigh::Solution.from_narray( h[:items] )
  end

  # @!visibility private
  def _dump *ignored
    Marshal.dump to_h
  end

  # @!visibility private
  def self._load buf
    h = Marshal.load buf
    from_h h
  end

  def save filename
    File.open( filename, 'wb' ) { |file| Marshal.dump( self, file ) }
  end

  def self.load filename
    File.open( filename, 'rb' ) { |file| Marshal.load(file) }
  end
end

class StolenSleigh::Trip
  # @!visibility private
  # Adds support for Marshal, via to_h and from_h methods
  def to_h
    Hash[
      :gifts => self.gifts
    ]
  end

  # @!visibility private
  # Constructs a StolenSleigh::Trip from hash description. Used internally to support Marshal.
  # @param [Hash] h Keys are :gifts
  # @return [StolenSleigh::Trip] new object
  def self.from_h h
    StolenSleigh::Trip.from_narray( h[:gifts] )
  end

  # @!visibility private
  def _dump *ignored
    Marshal.dump to_h
  end

  # @!visibility private
  def self._load buf
    h = Marshal.load buf
    from_h h
  end

  def save filename
    File.open( filename, 'wb' ) { |file| Marshal.dump( self, file ) }
  end

  def self.load filename
    File.open( filename, 'rb' ) { |file| Marshal.load(file) }
  end
end