# Linear-Regression
Secure MPC version of linear regression function; C code is based off of [this](https://www.github.com/samuelhavron/linear-regression) repository. Party 1 supplies independent ([x](testx.dat)) data points; party 2 supplies dependent ([y](testy.dat)) data points. Runtime information and code optimizations currently in development.

# Usage Notes
Does not compute values over 32,000 currently. Adjust implementation for long long ints if needed.

To compute data between remote hosts:
`make REMOTE_HOST=<ip_address>` then `./a.out <port> <1|2> <filename>`. If using an AMI through EC2 on AWS, add <port> as an inbound rule for each nodes' security group.
