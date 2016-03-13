#include<atomic_queue.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

static size_t ilog2Ceil_aux(size_t x) { return x>1?ilog2Ceil_aux((x+1)/2)+1:0; }

unsigned ilog2Ceil(size_t x) { return (unsigned)(x==0?-1:ilog2Ceil_aux(x)); }
unsigned bumpUp2Pow(size_t* x) 
{ unsigned l=ilog2Ceil(*x); 
  *x=(1<<l); 
  return l;
}

int atomic_queue_init(atomic_queue_t* qp,size_t maxElts,size_t eltSize)
{
  if(maxElts==0 || eltSize==0) return -EINVAL;
  atomic_queue_t q;
  *qp = q = malloc(sizeof(*q));
  q->eltSize = eltSize;
  q->logsize = bumpUp2Pow(&eltSize);
  q->loglen = bumpUp2Pow(&maxElts);
  q->buf=malloc(maxElts*q->eltSize);
  q->nxtRead=q->nxtWrite=0;
  int stat;
  if((stat=sem_init(&(q->usedSlots),0,0))!=0) return stat;
  if((stat=sem_init(&(q->emptySlots),0,maxElts))!=0) return stat;
  return 0;
}

void atomic_queue_release(atomic_queue_t q)
{
  free(q->buf);
  sem_destroy(&q->usedSlots);
  sem_destroy(&q->emptySlots);
  free(q);
}

// Not that errno values always make sense to the caller, 
//   but better than no error checking
// Assumes an int status local variable available for use
#define propagate_on_error(call) if((status=call)!=0) return status

int atomic_queue_write(atomic_queue_t q,const void* buf)
{
  int status;
  propagate_on_error(sem_wait(&q->emptySlots));
  size_t ind = (__sync_fetch_and_add(&q->nxtWrite,1)&((1<<q->loglen)-1))
                 << q->logsize;
  memcpy(q->buf+ind,buf,q->eltSize);
  propagate_on_error(sem_post(&q->usedSlots));
  return 0;
}
int atomic_queue_read(atomic_queue_t q,void* buf)
{
  int status;
  propagate_on_error(sem_wait(&q->usedSlots));
  size_t ind = (__sync_fetch_and_add(&q->nxtRead,1)&((1<<q->loglen)-1))
                  << q->logsize;
  memcpy(buf,q->buf+ind,q->eltSize);
  propagate_on_error(sem_post(&q->emptySlots));
  return 0;
}
