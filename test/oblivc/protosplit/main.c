#include<stdio.h>
#include<time.h>

#include"../common/util.h"
#include"protosplittest.h"

double lap;

int main(int argc,char* argv[])
{ 
  ProtocolDesc pd;

  if(argc<3)
  {
    if(argc<2) fprintf(stderr,"Port number missing\n");
    else if(argc<3) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <--|remote_host>\n",argv[0]);
    return 1;
  }

  const char* remote_host = (strcmp(argv[2],"--")==0?NULL:argv[2]);
  int i, party = (!remote_host?1:2);

  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);
  setCurrentParty(&pd,party);

  lap = wallClock();
  execYaoProtocol(&pd,goprotosplit,NULL);
  fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
  cleanupProtocol(&pd);
  fprintf(stderr,"\n");
  return 0;
}
