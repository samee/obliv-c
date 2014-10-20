#include<obliv.h>
#include<obliv_psi.h> // merge headers? TODO
#include<stdio.h>
#include<stdlib.h>

typedef struct {
  int party;
  const char* inputfile;
} cmdargs;

cmdargs parseCmd(int argc, char* argv[])
{
  cmdargs res;
  if(argc<2)
  { fprintf(stderr,"Usage: %s <party> <inputfile>\n",argv[0]);
    exit(1);
  }
  sscanf(argv[1],"%d",&res.party);
  res.inputfile=argv[2];
  return res;
}
/*
File format:
<maxlen>
data-element
data-element
data-element
...

Empty lines are ignored. Lines may have spaces. Newlines expected to end the
last line (unlike, e.g. DOS-formatted files)
Output data is meant to be fed directly into execPsiProtocol, so might not
always be null-terminated (e.g. if string length == max-length). But shorter
strings are always padded with nulls (so that equality tests do not involve
garbage data at the end).

Therefore, one way to guarantee null-terminated strings is to use
<maxlen> == 1 + max(over all i: strlen(data-element[i]))
*/
void parseFile(const char* filename,char** datap,size_t* np,size_t* lenp)
{
  FILE* fp;
  size_t ln=0,pn=16,len;
  char* data;
  if((fp=fopen(filename,"r"))==NULL)
  { fprintf(stderr,"Could not open input file\n");
    exit(2);
  }
  if(fscanf(fp,"%zu\n",&len)<0) exit(-1);
  data = malloc(pn*len+2);
  while(fgets(data+ln*len,len+2,fp))
  {
    char* rbuf = data+ln*len;
    size_t rlen = strlen(rbuf);
    if(rlen==0 || (rlen==1 && rbuf[0]=='\n')) continue; // blank line, ignore
    if(rbuf[rlen-1]=='\n') rlen--;
    else
    { fprintf(stderr,"Error: line too long\n");
      exit(2);
    }
    memset(rbuf+rlen,0,len-rlen);
    ln++;
    if(ln==pn)
    { pn*=2;
      data = realloc(data,pn*len+2);
    }
  }
  fclose(fp);
  data = realloc(data,ln*len);
  *datap = data;
  *np = ln;
  *lenp = len;
}

int main(int argc, char* argv[])
{
  cmdargs args = parseCmd(argc,argv);
  char *dataflat,**data;
  size_t i,n,len;
  parseFile(args.inputfile,&dataflat,&n,&len);
  data = malloc(n*sizeof(*data));
  for(i=0;i<n;++i) data[i]=dataflat+i*len;
  /*
  // Check parseFile()
  for(i=0;i<n;++i)
  { fwrite(dataflat+len*i,len,1,stderr);
    fputc('\n',stderr);
  }
  */
  ProtocolDesc pd;
  protocolUseStdio(&pd);
  setCurrentParty(&pd,args.party);
  // assumes both parties have inputs of equal size and len
  OcPsiResult* res = execPsiProtocol_DH(&pd,data,n,n,len);
  if(res->n==0) fprintf(stderr,"Nothing in common\n");
  else
  { fprintf(stderr,"# Elements in common = %d\n",res->n);
    for(i=0;i<res->n;++i)
      fprintf(stderr,"  %.*s\n",len,data[res->indices[i]]);
  }
  ocPsiResultRelease(res);
  cleanupProtocol(&pd);
  free(dataflat);
  free(data);
  return 0;
}
