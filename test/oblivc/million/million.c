#include<stdio.h>
#include<obliv.h>

#include"million.h"


int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  protocolIO io;
  if(argc<3)
  { if(argc<2) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"Wealth missing\n");
    fprintf(stderr,"Usage: %s <1|2> <wealth>\n",argv[0]);
    return 1;
  }

  // missing format check TODO
  sscanf(argv[2],"%d",&io.mywealth);
  protocolUseStdio(&pd);
  setCurrentParty(&pd,argv[1][0]=='1'?1:2);
  execYaoProtocol(&pd,millionaire,&io);
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: %d\n",io.cmp);
  return 0;
}
