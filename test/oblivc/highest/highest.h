#pragma once

#define MAXN 100

typedef struct protocolIO
{
  int i[100];
  int n;
  int res;
  
} protocolIO;

void highest(void* args);

const char* mySide();
