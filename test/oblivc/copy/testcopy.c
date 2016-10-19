#include"testcopy.h"
#include"../common/util.h"
#include<obliv.h>
#include<stdlib.h>
#include<stdio.h>

void showUsage(const char* exec)
{
  fprintf(stderr,"Parameters missing\n");
  fprintf(stderr,"Usage: %s <remote-server|--> <port> <numbers ...>\n",exec);
  exit(1);
}
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  TestCopyIO io;
  int i,me;
  const char* server;
  if(argc<3) showUsage(argv[0]);
  else
  { server=argv[1];
    me = (strcmp(server,"--")==0?1:2);
    if(me==1)
    { if(argc>=7) 
        for(i=0;i<4;++i) sscanf(argv[3+i],"%d",io.data+i);
      else showUsage(argv[0]);
    }
    else
    { if(argc>=5)
        for(i=0;i<2;++i) io.sel[i]=(argv[3+i][0]=='1');
      else showUsage(argv[0]);
    }
  }
  const char* remote_host = (me==1?NULL:server);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[2]);
  setCurrentParty(&pd,me);
  execYaoProtocol(&pd,testcopy,&io);
  printf("Result = %x,%x\n",io.result[0],io.result[1]);
  return 0;
}

