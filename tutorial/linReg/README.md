# Linear-regression
Development of [Obliv-C](https://www.github.com/samuelhavron/obliv-c/tree/linReg) and C linear regression program for developing secure multiparty computations.

To use, compile with `make` and run with `./linReg <filename>`. 
Run the [test data](test.data) with `./linReg test.data`. Or, make your own test data with `python gendat.py <filename> <number_of_points>`.

To contact me with any questions about this repository, open an issue on GitHub or email me at havron@virginia.edu.

# Usage Notes
Fixed point arithmetic is used in order to support Obliv-C. Note the small loss of accuracy due to fixed point arithmetic for a large input of data.

Dynamic resizing of data point arrays is used in order to allow for efficient processing of large data inputs.
