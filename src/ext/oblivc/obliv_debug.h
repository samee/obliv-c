#pragma once
#include<stdio.h>

void hexprint(const char* s,size_t len)
{
  size_t i;
  for(i=0;i<len;++i)
    fprintf(stderr,"%.02x ",0xff&s[i]);
  fprintf(stderr,"\n");
}
