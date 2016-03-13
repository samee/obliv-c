#pragma once
#include<semaphore.h>
#include<stdbool.h>

// What the heck! Why is there no small library already out there for this?

typedef struct
{
  char* buf;
  unsigned loglen,logsize;
  size_t eltSize;
  size_t nxtRead, nxtWrite; // uses GCC atomics
  sem_t usedSlots, emptySlots; // units of eltSize
} atomic_queue_s;

typedef atomic_queue_s* atomic_queue_t;

int atomic_queue_init(atomic_queue_t* q,size_t maxElts,size_t eltSize);
void atomic_queue_release(atomic_queue_t q);

int atomic_queue_write(atomic_queue_t q,const void* buf); // blocks if full
int atomic_queue_read(atomic_queue_t q,void* buf);       // blocks if empty
