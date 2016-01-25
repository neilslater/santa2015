require "rspec/core/rake_task"
require 'rake/extensiontask'

# Insert project local lib folder into library search path
$: << File.join( File.dirname( __FILE__ ), 'lib' )

desc "Unit tests"
RSpec::Core::RakeTask.new(:test) do |t|
  t.pattern = "spec/*_spec.rb"
  t.verbose = true
end

Rake::ExtensionTask.new do |ext|
  ext.name = 'stolen_sleigh'
  ext.source_pattern = "*.{c,h}"
  ext.ext_dir = 'ext/stolen_sleigh'
  ext.lib_dir = 'lib/stolen_sleigh'
end

desc "Import CSV"
task :import_csv do
  require 'stolen_sleigh'
  if StolenSleigh.ready_to_run?
    puts "CSV data already imported."
  elsif StolenSleigh.ready_to_import?
    StolenSleigh.import_from_csv
  else
    puts ''
    puts '**********************************************************'
    puts ' Place Kaggle file gifts.csv in data directory'
    puts '**********************************************************'
    puts ''
    raise "Data file missing: #{StolenSleigh::CSV_PATH}"
  end
end

desc "Run solver and save submission"
task :solve do
  require 'stolen_sleigh'
  unless StolenSleigh.ready_to_run?
    raise "Not ready, import CSV data first."
  end
  solver = StolenSleigh::Solver.new
  score = solver.run
  submission_file = File.join( StolenSleigh::OUTPUT_PATH, "submission_#{score.to_i}.csv" )
  puts "Writing submission file: #{submission_file}"
  solver.solution.write_csv( submission_file )
end

task :default => [:compile, :import_csv, :test, :solve]
