PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)
testName=testFloatOps
include ../common/Makefile.simple

# Run tests quickly
t1:
	$(./a.out)
	./a.out $(CONNECTION) 1

t2:
	$(./a.out) 
	./a.out $(CONNECTION) 2
