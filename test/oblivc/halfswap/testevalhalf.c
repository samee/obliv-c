#include"testevalhalf.h"
#include"util.h"
#include<obliv.h>
#include<stdlib.h>
#include<stdio.h>

void showUsage(const char* exec)
{
  fprintf(stderr,"Parameters missing\n");
  fprintf(stderr,"Usage: %s -- <port> <numbers ...>\n",exec);
  fprintf(stderr,"       %s <remote-server> <port> <0 1 ...>\n",exec);
  exit(1);
}
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  TestEvalHalfIO io;
  int i,me;
  const char* server;
  if(argc<3) showUsage(argv[0]);
  else
  { server=(strcmp(argv[1],"--")?argv[1]:NULL);
    me = (server?2:1);
    io.n = argc-3;
    io.data = malloc(sizeof(int)*2*io.n);
    io.sel = malloc(sizeof(bool)*io.n);
    for(i=0;i<io.n;++i) 
      if(sscanf(argv[3+i],"%d",io.data+i)!=1)
        showUsage(argv[0]);
    if(me==1) io.n/=2;
    if(me==2) for(i=0;i<io.n;++i) io.sel[i]=io.data[i];
  }
  ocTestUtilTcpOrDie(&pd,server,argv[2]);
  setCurrentParty(&pd,me);
  execYaoProtocol(&pd,testEvalHalf,&io);
  printf("Result:");
  for(i=0;i<2*io.n;++i)
    printf(" %d",io.data[i]);
  printf("\n");
  free(io.data);
  free(io.sel);
  return 0;
}

