# StolenSleigh

Ruby/C solver for Kaggle Christmas competition *Santa's Stolen Sleigh* from December 2015.

For competition details, and problem data (which you will need to fetch separately to run this code),
see https://www.kaggle.com/c/santas-stolen-sleigh

This code is based on, but not identical to, the solver for ninth place position. The differences
include a lot of code tidying and simplification. As a result of this, the eventual solution arrived
at by this code is not exactly the same as the submission I made for ninth place.

## Installation

 * You need a Ruby interpreter, recommended version 2.1.4 or higher, with support for compiling native extensions.

 * Copy this repo locally, and make the directory with the README.md your current directory

 * Install dependencies:


    $ bundle install


 * Place a copy of ```gifts.csv``` in the ```data``` folder.


## Running the solver

### Compile, import gift data, test code and run solution:

    $ rake

This should compile the C code, import the problem data from the CSV file (into Ruby's NArray
format), run the unit tests, and then run the default solution. The process can take several days
in total, although a reasonable score, which would earn a rank of about 35, can be obtained in a few hours.
The output is saved to ```output/submission_<score>.csv```.

You may need to use ```bundle exec rake``` instead of just ```rake``` if you have other Ruby projects installed that
use older versions of one of the dependencies. However, I have not seen that in practice.

### Scripted tasks

For a full list of tasks supported:

    $ rake -T

### Extending the code

The Ruby object model is fully documented and can be used to construct variations of the solvers or explore existing
solutions.

## Contributing

1. Fork it ( https://github.com/slobo777/stolen_sleigh/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
