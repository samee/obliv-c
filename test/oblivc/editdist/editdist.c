#include<stdio.h>
#include<obliv.h>
#include<memory.h>
#include<time.h>

#include"editdist.h"


int currentParty;
const char* mySide()
{
  if(currentParty==1) return "Generator";
  else return "Evaluator";
}


double wallClock()
{
  struct timespec t;
  //clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

double lap;

int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  protocolIO io;
  if(argc<3)
  { if(argc<2) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <1|2> <string>\n",argv[0]);
    return 1;
  }

  io.s = argv[2];
  io.n = strlen(argv[2]);
  if(io.n>MAXN) 
  { fprintf(stderr,"string too big\n");
    return 1;
  }

  protocolUseStdio(&pd);
  currentParty = (argv[1][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  lap = wallClock();
  execYaoProtocol(&pd,editDistance,&io);
  fprintf(stderr,"%s total time: %lf s\n",mySide(),wallClock()-lap);
  fprintf(stderr,"%u\n",yaoGateCount());
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: %d\n",io.res);
  return 0;
}
