#include<stdio.h>
#include<obliv.h>
#include<memory.h>

#include"../common/util.h"
void test(void * args);
// Usage: ./a.out <port> <--|server>
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  //protocolUseStdio(&pd);
  const char* remote_host = (strcmp(argv[2],"--")?argv[2]:NULL);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);

  int currentParty = (remote_host?2:1);
  setCurrentParty(&pd,currentParty);
  execYaoProtocol(&pd,test,0);
  cleanupProtocol(&pd);
  return 0;
}
