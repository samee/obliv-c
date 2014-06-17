#include<stdio.h>
#include<stdlib.h>
#include<obliv.h>
#include<string.h>
#include<time.h>

#include "mutual.h"

int currentParty;

/* Function to merge the two haves arr[l..m] and arr[m+1..r] of array arr[] */
void merge(char arr[MAXN][MAXL], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* create temp arrays */
    char L[n1][100], R[n2][100];
 
    /* Copy data to temp arrays L[] and R[] */
    for(i = 0; i < n1; i++)
        strcpy(L[i], arr[l+i]);
    for(j = 0; j < n2; j++)
        strcpy(R[j], arr[m+1+j]);
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (strcmp(L[i],R[j]) < 0)
        {
            strcpy(arr[k], L[i]);
            i++;
        }
        else
        {
            strcpy(arr[k], R[j]);
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there are any */
    while (i < n1)
    {
        strcpy(arr[k], L[i]);
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there are any */
    while (j < n2)
    {
        strcpy(arr[k], R[j]);
        j++;
        k++;
    }
}
 
/* l is for left index and r is right index of the sub-array
  of arr to be sorted */
void mergeSort(char arr[MAXN][MAXL], int l, int r)
{
    if (l < r)
    {
      int m = (l+r) / 2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }
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
  }
  io.size = i;

  mergeSort(io.mine, 0, io.size-1);

  fprintf(stderr,"Result: %d\n", io.size);
  
  fclose(infile);

  //standard setup
  protocolUseStdio(&pd);
  currentParty = (argv[1][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  setCurrentParty(&pd,currentParty);
  lap = wallClock();
  //execYaoProtocol(&pd,mutualFriends, &io);
  //execDebugProtocol(&pd,mutualFriends, &io);
    execYaoProtocol(&pd,sortMutual, &io);
  //execDebugProtocol(&pd,sortMutual, &io);
  fprintf(stderr,"%s total time: %lf s\n",mySide(),wallClock()-lap);
  fprintf(stderr,"Gate Count: %u\n",yaoGateCount());
  cleanupProtocol(&pd);
  fprintf(stderr, "Result: %d %c\n", io.commonSize, io.common[0][0]);
  for(i=0; i<MAXN; i++)
    if(io.common[i][0]!='\0')
      fprintf(stderr, "%s", io.common[i]);
  return 0;
}
