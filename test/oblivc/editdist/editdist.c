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
    else if(argc<3) fprintf(stderr,"Server location missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <--|remote_host> <string>\n",argv[0]);
    return 1;
  }

  io.s = argv[3];
  io.n = strlen(argv[3]);
  if(io.n>MAXN) 
  { fprintf(stderr,"string too big\n");
    return 1;
  }

  //protocolUseStdio(&pd);
  const char* remote_host = (strcmp(argv[2],"--")?argv[2]:NULL);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);

  currentParty = (remote_host?2:1);
  setCurrentParty(&pd,currentParty);
  lap = wallClock();
  execYaoProtocol(&pd,editDistance,&io);
  fprintf(stderr,"%s total time: %lf s\n",mySide(),wallClock()-lap);
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: %d\n",io.res);
  return 0;
}
