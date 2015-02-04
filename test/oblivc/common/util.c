#include<obliv.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"util.h"

#ifndef REMOTE_HOST
#define REMOTE_HOST localhost
#endif

#define QUOTE(x) #x
#define TO_STRING(x) QUOTE(x)
static const char remote_host[] = TO_STRING(REMOTE_HOST);
#undef TO_STRING
#undef QUOTE

double wallClock()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

void ocTestUtilTcpOrDie(ProtocolDesc* pd,bool isServer,const char* port)
{
  if(isServer)
  { if(protocolAcceptTcp2P(pd,port)!=0)
    { fprintf(stderr,"TCP accept failed\n");
      exit(1);
    }
  }
  else 
    if(protocolConnectTcp2P(pd,remote_host,port)!=0) 
    { fprintf(stderr,"TCP connect failed\n");
      exit(1);
    }
}
