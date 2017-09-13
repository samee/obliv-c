#include<stdio.h>
#include<time.h>

#include"../common/util.h"
#include"protosplittest.h"

double lap;

int main(int argc,char* argv[])
{ 
  ProtocolDesc pd;

  if(argc<5)
  {
    if(argc<2) fprintf(stderr,"Port number missing\n");
    else if(argc<3) fprintf(stderr,"Party missing\n");
    else if(argc<4) fprintf(stderr,"Element Count missing\n");
    else fprintf(stderr,"Thread Count missing\n");
    fprintf(stderr,"Usage: %s <port> <--|remote_host> elements threads\n",argv[0]);
    return 1;
  }

  const char* remote_host = (strcmp(argv[2],"--")==0?NULL:argv[2]);
  int i, party = (!remote_host?1:2);

  struct args a = {
    .threads=atoi(argv[4]), .elct=atoi(argv[3])
  };

  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);
  setCurrentParty(&pd,party);

  lap = wallClock();
  execYaoProtocol(&pd,goprotosplit,&a);
  fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
  cleanupProtocol(&pd);
  fprintf(stderr,"\n");
  return 0;
}
