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
  // Usage:
  //   ./a.out -- 1234 &
  //   ./a.out localhost 1234
  // The first command starts out a background server listening on port 1234.
  // The second command connects to this server, which is localhost at port 1234
  extern void randomPsi(void*);
  ProtocolDesc pd;
  //protocolUseStdio(&pd);
  const char* remote_host = (strcmp(argv[2],"--")?argv[2]:NULL);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);
  lap = wallClock();
  setCurrentParty(&pd,remote_host?2:1);
  execYaoProtocol(&pd,randomPsi,NULL);
  fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
  cleanupProtocol(&pd);
  return 0;
}
int main(int argc,char* argv[])
{
  return testPsiMain(argc,argv);
}
