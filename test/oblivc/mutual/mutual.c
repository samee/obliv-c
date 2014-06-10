#include<stdio.h>
#include<stdlib.h>
#include<obliv.h>
#include<string.h>
#include<time.h>

#include "mutual.h"

int currentParty;

void merge(int start, int end, int mid, char* list){
  
}

void mergesort(int start, int end, char* list){
  if(start>=end)
    return;
  int mid = (start + end) / 2;
  mergesort(start, mid, list);
  mergesort(mid+1, end, list);
  merge(start, end, mid, list);
}

const char* mySide()
{
  if(currentParty==1) return "Generator";
  else return "Evaluator";
}

double wallClock()
{
  struct timespec t;
  int i = clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}
double lap;

int main(int argc,char *argv[])
{
  //these variables are used everytime
  ProtocolDesc pd;
  protocolIO io;
  
  //checking that the input has the correct number of variables
  if(argc<3)
  { if(argc<2) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"friends missing\n");
    fprintf(stderr,"Usage: %s <1|2> <string>\n",argv[0]);
    return 1;
  }
  FILE *infile;
  infile = fopen(argv[2], "r");
  if(infile==NULL){
    //Error("Unable to open file");
    fprintf(stderr, "ERROR!");
    return 1;
  }
  //TODO change input file so that it says how many friends there are
  int i=0;
  char buf[MAXN][MAXL];
  while(fgets(buf[i], MAXL, infile)!=NULL){
    strcpy(io.mine[i], buf[i]);
    i++;
    //buf[i] = malloc(20*sizeof(char));
  }
  io.size = i;
  fprintf(stderr,"Result: %d\n", io.size);
  
  fclose(infile);
  //standard setup
  protocolUseStdio(&pd);
  currentParty = (argv[1][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  setCurrentParty(&pd,currentParty);
  lap = wallClock();
  execYaoProtocol(&pd,mutualFriends,&io);
  //execDebugProtocol(&pd,mutualFriends, &io);
  fprintf(stderr,"%s total time: %lf s\n",mySide(),wallClock()-lap);
  fprintf(stderr,"Gate Count: %u\n",yaoGateCount());
  cleanupProtocol(&pd);
  fprintf(stderr, "Result: %d %c\n", io.commonSize, io.common[0][0]);
  for(i=0; i<MAXN; i++)
    //if(io.common[i][0]!='\0')
      fprintf(stderr, "%s", io.common[i]);
  return 0;
}
