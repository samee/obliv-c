#include"testgenhalf.h"
#include"util.h"
#include<obliv.h>
#include<stdlib.h>
#include<stdio.h>

void showUsage(const char* exec)
{
  fprintf(stderr,"Parameters missing\n");
  fprintf(stderr,"Usage: %s -- <port> <0 1 ....>\n",exec);
  fprintf(stderr,"       %s <remote-server> <port> <numbers ...>\n",exec);
  exit(1);
}
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  TestGenHalfIO io;
  int i,me;
  const char* server;
  if(argc<3) showUsage(argv[0]);
  else
  { server = (strcmp(argv[1],"--")?argv[1]:NULL);
    me = (server?2:1);
    io.n = argc-3;
    io.data = malloc(sizeof(int)*2*io.n);
    for(i=0;i<io.n;++i) 
      if(sscanf(argv[3+i],"%d",io.data+i)!=1)
        showUsage(argv[0]);
    if(me==2) io.n/=2;
    
  }
  ocTestUtilTcpOrDie(&pd,server,argv[2]);
  setCurrentParty(&pd,me);
  execYaoProtocol(&pd,testGenHalf,&io);
  printf("Result:");
  for(i=0;i<2*io.n;++i)
    printf(" %d",io.data[i]);
  printf("\n");
  free(io.data);
  return 0;
}

