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
    else if(argc<4) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <proto> <1|2> <string>\n",argv[0]);
    return 1;
  }

  io.s = argv[4];
  io.n = strlen(argv[4]);
  if(io.n>MAXN) 
  { fprintf(stderr,"string too big\n");
    return 1;
  }

  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,argv[3][0]=='1',argv[1]);

  setCurrentParty(&pd,(argv[3][0]=='1'?1:2));
  if(!strcmp("yao",argv[2])) execYaoProtocol(&pd,hammingDistance,&io);
  else execNpProtocol_Bcast1(&pd,hammingDistance,&io);
  fprintf(stderr,"Result: %d\n",io.res);
  cleanupProtocol(&pd);
  return 0;
}
