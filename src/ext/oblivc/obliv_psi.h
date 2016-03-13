#pragma once
#include<obliv_types.h>

typedef struct OcPsiResult
{ int n;
  int *indices;
} OcPsiResult;

void ocPsiResultRelease(OcPsiResult* res);
OcPsiResult* execPsiProtocol_DH     (ProtocolDesc* pd,
                  char** data,int ni,int nu,int len);
OcPsiResult* execPsiProtocol_allPair(ProtocolDesc* pd,
                  char** data,int ni,int nu,int len);
