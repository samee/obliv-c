#include<stdio.h>
#include<obliv.h>

#include"hamming.h"
#include"../common/util.h"


int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  protocolIO io;
  if(argc<5)
  { if(argc<2) fprintf(stderr,"Port number missing\n");
    else if(argc<3) fprintf(stderr,"Protocol ID missing\n");
    else if(argc<4) fprintf(stderr,"Remote server missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <proto> <--|server> <string>\n",argv[0]);
    return 1;
  }

  io.s = argv[4];
  io.n = strlen(argv[4]);
  if(io.n>MAXN) 
  { fprintf(stderr,"string too big\n");
    return 1;
  }

  //protocolUseStdio(&pd);
  const char* remote_host = (strcmp(argv[3],"--")==0?NULL:argv[3]);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);

  setCurrentParty(&pd,(remote_host?2:1));
  if(!strcmp("yao",argv[2])) execYaoProtocol(&pd,hammingDistance,&io);
  else execNpProtocol_Bcast1(&pd,hammingDistance,&io);
  fprintf(stderr,"Result: %d\n",io.res);
  cleanupProtocol(&pd);
  return 0;
}
