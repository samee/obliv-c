#include<obliv.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"util.h"

double wallClock()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

void ocTestUtilTcpOrDie(ProtocolDesc* pd,const char* remote_host,
                        const char* port)
{
  if(!remote_host)
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
