OBLIVCC=../../../bin/oblivcc
RUNTIME=../../../_build/libobliv.a
REMOTE_HOST=localhost
CFLAGS=-DREMOTE_HOST=$(REMOTE_HOST) -O3

.PHONY: all

all: testgenhalf testevalhalf

testgenhalf: testgenhalf.oc testgenhalf.c testgenhalf.h $(RUNTIME)
	$(OBLIVCC) $(CFLAGS) -I . -I ../common testgenhalf.oc testgenhalf.c ../common/util.c -o $@

testevalhalf: testevalhalf.oc testevalhalf.c testevalhalf.h $(RUNTIME)
	$(OBLIVCC) $(CFLAGS) -I . -I ../common testevalhalf.oc testevalhalf.c ../common/util.c -o $@

