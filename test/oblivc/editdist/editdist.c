#include<stdio.h>
#include<obliv.h>
#include<memory.h>

#include"editdist.h"
#include"../common/util.h"


int currentParty;
const char* mySide()
{
  if(currentParty==1) return "Generator";
  else return "Evaluator";
}

double lap;

int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  protocolIO io;
  if(argc<4)
  { if(argc<2) fprintf(stderr,"Port number missing\n");
    else if(argc<3) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <1|2> <string>\n",argv[0]);
    return 1;
  }

  io.s = argv[3];
  io.n = strlen(argv[3]);
  if(io.n>MAXN) 
  { fprintf(stderr,"string too big\n");
    return 1;
  }

  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,argv[2][0]=='1',argv[1]);

  currentParty = (argv[2][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  lap = wallClock();
  execYaoProtocol(&pd,editDistance,&io);
  fprintf(stderr,"%s total time: %lf s\n",mySide(),wallClock()-lap);
  fprintf(stderr,"%u\n",yaoGateCount());
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: %d\n",io.res);
  return 0;
}
