#pragma once
#include<stdbool.h>

typedef struct
{
  int n;
  int* data;
  bool* sel;
}TestEvalHalfIO ;

void testEvalHalf(void* vargs);
