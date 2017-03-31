CILPATH=../../../
OBLIVCH=$(CILPATH)/src/ext/oblivc
OBLIVCC= $(CILPATH)/bin/oblivcc
OBLIVCA= $(CILPATH)/_build/libobliv.a
REMOTE_HOST=localhost
CFLAGS=-DREMOTE_HOST=$(REMOTE_HOST) -O3  -fopenmp -I .

./a.out: protosplittest.oo protosplittest.o main.o ../common/util.o
	$(OBLIVCC) $(OBLIVCA) $^ -lm -lgomp

%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o -I $(OBLIVCH)

%.oo: %.oc
	$(OBLIVCC) -c $(CFLAGS) $*.oc -o $*.oo

clean:
	rm -f a.out
