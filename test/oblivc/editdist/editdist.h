#pragma once

#define MAXN 200

typedef struct protocolIO
{ const char* s;
  int n;
  int res;
} protocolIO;

void editDistance(void* args);

// Generic utilities
extern double lap;
double wallClock();
const char* mySide();
