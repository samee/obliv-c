PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)
TX=tx.dat
TY=ty.dat
testName=linreg
include ../common/Makefile.simple
CFLAGS=-DREMOTE_HOST=$(REMOTE_HOST) -O3 -lm

# Run tests quickly
t1:
	$(./a.out)
	./a.out $(CONNECTION) 1 $(TX)

t2:
	$(./a.out) 
	./a.out $(CONNECTION) 2 $(TY)
