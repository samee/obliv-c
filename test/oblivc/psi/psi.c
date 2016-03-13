#include<stdio.h>
#include<obliv.h>

#include<../common/util.h>

int testSorterMain(int argc,char* argv[])
{
  ProtocolDesc pd;
  extern void testSorter(void*);
  extern bool testSorterResult;
  protocolUseStdio(&pd);
  setCurrentParty(&pd,argv[1][0]=='1'?1:2);
  execDebugProtocol(&pd,testSorter,NULL);
  cleanupProtocol(&pd);
  if(testSorterResult) fprintf(stderr,"Sorted!\n");
  else fprintf(stderr,"Unsorted mess!\n");
  return 0;
}

double lap;
int testPsiMain(int argc,char* argv[])
{
  extern void randomPsi(void*);
  ProtocolDesc pd;
  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,argv[2][0]=='1',argv[1]);
  lap = wallClock();
  setCurrentParty(&pd,argv[2][0]=='1'?1:2);
  execYaoProtocol(&pd,randomPsi,NULL);
  fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
  cleanupProtocol(&pd);
  return 0;
}
int main(int argc,char* argv[])
{
  return testPsiMain(argc,argv);
}
