#include <obliv.h>
#include <stdint.h>
#include <stdbool.h>

#define ELCT 2000

typedef void (* split_fn)(void *, uint32_t *, int, void *);

void parallelize(split_fn fn, void * output1, void * output2, void * output3, uint32_t * input, int leneach, void * pd1, void* pd2);
void goprotosplit(void* vargs);