#include<stdio.h>
#include<obliv.h>
#include<memory.h>

#include"../common/util.h"
void test(void * args);
int main(int argc,char *argv[])
{
  ProtocolDesc pd;
  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,argv[2][0]=='1',argv[1]);

  int currentParty = (argv[2][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  execYaoProtocol(&pd,test,0);
  cleanupProtocol(&pd);
  return 0;
}
