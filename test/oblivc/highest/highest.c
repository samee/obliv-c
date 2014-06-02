#include<stdio.h>
#include<obliv.h>

#include "highest.h"

int currentParty;

const char* mySide()
{
  if(currentParty==1) return "Generator";
  else return "Evaluator";
}

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
  io.n = argc-2;
 
  int g = atoi("15");
  int j;
  for(j = 0; j<io.n; j++){
    io.i[j] = atoi(argv[j+2]);
  }
  protocolUseStdio(&pd);
  currentParty = (argv[1][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  execYaoProtocol(&pd,highest,&io);
  fprintf(stderr,"%u\n",yaoGateCount());
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: %d\n",io.res);
  return 0;
}
