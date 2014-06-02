#include<stdio.h>
#include<obliv.h>

#include "highest.h"

int currentParty;

const char* mySide()
{
  if(currentParty==1) return "Generator";
  else return "Evaluator";
}

int main(int argc,char *argv[])
{
  //these variables are used everytime
  ProtocolDesc pd;
  protocolIO io;
  
  //checking that the input has the correct number of variables
  if(argc<3)
  { if(argc<2) fprintf(stderr,"Party missing\n");
    else fprintf(stderr,"string missing\n");
    fprintf(stderr,"Usage: %s <1|2> <string>\n",argv[0]);
    return 1;
  }
  
  //sets the size of the array
  io.n = argc-2;
  //the the values in the array to the input values
  int j;
  for(j = 0; j<io.n; j++){
    io.i[j] = atoi(argv[j+2]);
  }
  //these steps are done everytime to set-up the oc file
  protocolUseStdio(&pd);
  currentParty = (argv[1][0]=='1'?1:2);
  setCurrentParty(&pd,currentParty);
  execYaoProtocol(&pd,highest,&io);
  cleanupProtocol(&pd);
  //prints out the result
  fprintf(stderr,"Result: %d\n",io.res);
  return 0;
}
