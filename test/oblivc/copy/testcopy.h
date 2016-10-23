#pragma once
#include<stdbool.h>

typedef struct 
{
  int data[4];
  bool sel[2];
  int result[2];
} TestCopyIO;

void testcopy(void* vargs);
