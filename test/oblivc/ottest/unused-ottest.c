#define ARRSIZE(a) (sizeof(a)/sizeof(*a))
#define XBYTES 6

#include<stdio.h>
#include<obliv_common.h> // for npotRecverNew()/npotSenderNew()
#include<bcrandom.h>
#include<time.h>

typedef struct NpotSender NpotSender;
typedef struct NpotRecver NpotRecver;

double wallClock()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

void clockExtension(int argc, char* argv[])
{
  int i,n;
  ProtocolDesc pd;

  protocolUseStdio(&pd);
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }

  if(argv[1][0]=='R')
  {
    struct HonestOTExtRecver* r;
    char *buf;
    bool *sel;
    int i,n;
    clock_t lap;
    double wlap;
    setCurrentParty(&pd,2);
    sscanf(argv[2],"%d",&n);
    buf = malloc(11*n); sel = malloc(n);
    for(i=0;i<n;++i) sel[i]=i%2;
    lap = clock();
    wlap = wallClock();
    r = honestOTExtRecverNew(&pd,1);
    honestOTExtRecv1Of2(r,buf,sel,n,11);
    honestOTExtRecverRelease(r);
    fprintf(stderr,"R CPU time is %lf, wall time is %lf\n",
        (clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
    char ref[11];
    for(i=0;i<n;++i)
    { snprintf(ref,11,"%d %d",sel[i],i);
      if(strncmp(ref,buf+i*11,11)) fprintf(stderr,"Didn't work. Got '%s'\n",buf+i*11);
    }
    free(buf); free(sel);
  }else
  { struct HonestOTExtSender* s;
    char *buf0,*buf1;
    int i,n=10,bs;
    clock_t lap;
    double wlap;
    setCurrentParty(&pd,1);
    sscanf(argv[2],"%d",&n);
    buf0 = malloc(11*n); buf1 = malloc(11*n);
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    lap = clock();
    wlap = wallClock();
    s = honestOTExtSenderNew(&pd,2);
    honestOTExtSend1Of2(s,buf0,buf1,n,11);
    honestOTExtSenderRelease(s);
    fprintf(stderr,"S CPU time is %lf, wall time is %lf\n",
        (clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
    free(buf0); free(buf1);
  }
}
void clock1Of2(int argc, char* argv[])
{
  int i,n;
  ProtocolDesc pd;

  protocolUseStdio(&pd);
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }

  if(argv[1][0]=='R')
  {
    NpotRecver* r;
    char *buf,*sel;
    int i,n,bs;
    clock_t lap;
    double wlap;
    setCurrentParty(&pd,2);
    sscanf(argv[2],"%d",&n);
    buf = malloc(11*n); sel = malloc(n);
    for(i=0;i<n;++i) sel[i]=i%2;
    for(bs=1;bs<=16;++bs)
    { lap = clock();
      wlap = wallClock();
      r = npotRecverNew(1<<bs,&pd,1);
      npotRecv1Of2(r,buf,sel,n,11,bs);
      npotRecverRelease(r);
      fprintf(stderr,"R For batch size %d, CPU time is %lf, wall time is %lf\n",
          bs,(clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
      char ref[11];
      for(i=0;i<n;++i)
      { snprintf(ref,11,"%d %d",sel[i],i);
        if(strncmp(ref,buf+i*11,11)) fprintf(stderr,"Didn't work. Got '%s'\n",buf+i*11);
      }
    }
    free(buf); free(sel);
  }else
  { NpotSender* s;
    char *buf0,*buf1;
    int i,n=10,bs;
    clock_t lap;
    double wlap;
    setCurrentParty(&pd,1);
    sscanf(argv[2],"%d",&n);
    buf0 = malloc(11*n); buf1 = malloc(11*n);
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    for(bs=1;bs<=16;++bs)
    { lap = clock();
      wlap = wallClock();
      s = npotSenderNew(1<<bs,&pd,2);
      npotSend1Of2(s,buf0,buf1,n,11,bs);
      npotSenderRelease(s);
      fprintf(stderr,"S For batch size %d, CPU time is %lf, wall time is %lf\n",
          bs,(clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
    }
    free(buf0); free(buf1);
  }
}
void testExtension(int argc, char* argv[])
{
  int i,n;
  ProtocolDesc pd;
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }
  protocolUseStdio(&pd);

  if(argv[1][0]=='R')
  {
    struct HonestOTExtRecver* r;
    char buf[11*20];
    bool sel[20];
    int i,n;
    setCurrentParty(&pd,2);
    n = argc-2;
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) sel[i]=(argv[i+2][0]=='1');
    r = honestOTExtRecverNew(&pd,1);
    honestOTExtRecv1Of2(r,buf,sel,n,11);
    honestOTExtRecverRelease(r);
    for(i=0;i<n;++i) fprintf(stderr,"Element %d: %s\n",i,buf+11*i);
  }else
  { struct HonestOTExtSender* s;
    char buf0[11*20], buf1[11*20];
    int i,n=10;
    setCurrentParty(&pd,1);
    sscanf(argv[2],"%d",&n);
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    s = honestOTExtSenderNew(&pd,2);
    honestOTExtSend1Of2(s,buf0,buf1,n,11);
    honestOTExtSenderRelease(s);
  }
}
void test1Of2(int argc, char* argv[])
{
  int i,n;
  ProtocolDesc pd;
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }
  protocolUseStdio(&pd);

  if(argv[1][0]=='R')
  {
    NpotRecver* r;
    char buf[11*20],sel[20];
    int i,n;
    setCurrentParty(&pd,2);
    n = argc-2;
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) sel[i]=(argv[i+2][0]=='1');
    r = npotRecverNew(8,&pd,1);
    npotRecv1Of2(r,buf,sel,n,11,4);
    npotRecverRelease(r);
    for(i=0;i<n;++i) fprintf(stderr,"Element %d: %s\n",i,buf+11*i);
  }else
  { NpotSender* s;
    char buf0[11*20], buf1[11*20];
    int i,n=10;
    setCurrentParty(&pd,1);
    sscanf(argv[2],"%d",&n);
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    s = npotSenderNew(8,&pd,2);
    npotSend1Of2(s,buf0,buf1,n,11,4);
    npotSenderRelease(s);
  }
}

void testBaseLarge(int argc, char* argv[])
{
  const int len = 80;
  int n,i;
  ProtocolDesc pd;
  if(argc<2
      || (argv[1][0]=='R' && argc<4)
      || (argv[1][0]=='S' && argc<3)) { 
    fprintf(stderr,"Too few parameters\n"); 
    return; 
  }
  
  protocolUseStdio(&pd);
  sscanf(argv[2],"%d",&n);
  if(n<1) n=1;

  if(argv[1][0]=='S')
  {
    char** buf;
    NpotSender* s;
    buf=malloc(sizeof(char*)*n);
    setCurrentParty(&pd,1);
    for(i=0;i<n;++i)
    { buf[i]=malloc(len*sizeof(char));
      sprintf(buf[i],"Item lsdflkdfndslkfsdlkdfdlkf"
          "dlkfldsmfkldsfsdfldsfdslkfjsdklf %d",i+5);
    }
    s = npotSenderNew(4,&pd,2);
    npotSendLong(s,buf,n,len);
    npotSenderRelease(s);
    for(i=0;i<n;++i) free(buf[i]);
    free(buf);
  }else
  {
    char* buf;
    NpotRecver* r;
    buf = malloc(sizeof(char)*len);
    setCurrentParty(&pd,2);
    r = npotRecverNew(4,&pd,1);
    sscanf(argv[3],"%d",&i);
    if(i<0 || i>=n) { fprintf(stderr,"index out of range\n"); return; }
    npotRecvLong(r,buf,i,n,len);
    buf[len-1]=0;
    fprintf(stderr,"Data received: %s\n",buf);
    npotRecverRelease(r);
    free(buf);
  }
}
void testBaseFour(int argc, char* argv[])
{
  int n = 4;
  ProtocolDesc pd;
  protocolUseStdio(&pd);
  if(argc<=2 || argv[1][0]=='S')
  { char* buf[] = {"Hello", "World", "Seven", "More!" };
    setCurrentParty(&pd,1);
    NpotSender* s = npotSenderNew(n,&pd,2);
    npotSend(s,buf,n,XBYTES);
    npotSenderRelease(s);
  }else
  { char buf[XBYTES];
    setCurrentParty(&pd,2);
    NpotRecver* r = npotRecverNew(n,&pd,1);
    npotRecv(r,buf,argv[2][0]-'0',n,XBYTES);
    npotRecverRelease(r);
    buf[XBYTES-1]=0;
    fprintf(stderr,"Data received: %s\n",buf);
  }
}

void showhex(const unsigned char* c,size_t len)
{
  size_t i;
  for(i=0;i<len;++i) fprintf(stderr,"%02x",c[i]);
  fprintf(stderr,"\n");
}

int main(int argc, char* argv[])
{
  dhRandomInit();
  clockExtension(argc,argv);
  dhRandomFinalize();
  return 0;
}
