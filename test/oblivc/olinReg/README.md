# Linear-Regression
Secure MPC version of linear regression function using [Obliv-c language](www.oblivc.org); C code is based off of [this](https://www.github.com/samuelhavron/linear-regression) repository. Party 1 supplies independent ([x](testx.dat)) data points; party 2 supplies dependent ([y](testy.dat)) data points. Runtime information and code optimizations currently in development. Sample tests using publicly released datasets currently in development.

# Usage Notes
Random test data can be generated with `python gendat.py tx.dat ty.dat txy.dat <number_of_points>`. To quickly run the test data over a local network, targets t1 and t2 are included in the [Makefile](Makefile). Run `make` on both shells, then `make t1` on shell with independent (x) points and then `make t2` on shell with dependent (y) points.

Does not compute data values over 32,000 currently. Adjust implementation for long long ints if needed.
     
If using EC2 on AWS to compute data:
Add desired TCP port as a custom inbound rule to each AMI nodes' security group before running. Use Public IP or DNS name for hostname in runtime argument.