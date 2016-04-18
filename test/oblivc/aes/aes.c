#include<obliv.h>
#include<stdio.h>
#include<time.h>

#include"../common/util.h"
#include"aes.h"

double lap;

int main(int argc,char* argv[])
{ 
  ProtocolDesc pd;
  protocolIO io;

  if(argc<4)
  {
    if(argc<2) fprintf(stderr,"Port number missing\n");
    else if(argc<3) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <port> <--|remote_host> <string>\n",argv[0]);
    fprintf(stderr,"  Server provides key in hex\n");
    fprintf(stderr,"  Client provides plaintext in hex\n");
    return 1;
  }

  const char* remote_host = (strcmp(argv[2],"--")==0?NULL:argv[2]);
  int i, party = (!remote_host?1:2);

  //protocolUseStdio(&pd);
  ocTestUtilTcpOrDie(&pd,remote_host,argv[1]);
  if(party==1) io.testkey=argv[3];
  else io.testplain=argv[3];
  setCurrentParty(&pd,party);

  lap = wallClock();
  execYaoProtocol(&pd,goaes,&io);
  fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
  cleanupProtocol(&pd);
  fprintf(stderr,"Result: ");
  for(i=0;i<16;++i) fprintf(stderr,"%02x",io.testcipher[i]&0xff);
  fprintf(stderr,"\n");
  return 0;
}
