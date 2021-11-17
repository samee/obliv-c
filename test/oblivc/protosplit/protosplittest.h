#include <obliv.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef void (* split_fn)(void *, uint32_t *, size_t, size_t, void *);

struct args {
  size_t threads;
  size_t elct;
};

void parallelize(split_fn fn, void * output, uint32_t * input, size_t leneach, size_t threads, void * pds);
void goprotosplit(void* vargs);