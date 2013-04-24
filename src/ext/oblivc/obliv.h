#ifndef OBLIV_H
#define OBLIV_H

#include<obliv_types.h>

// This header should only have functions directly used by the user,
//   not functions that will appear in the generated C code.
//   Moreover, including this function in an obliv-C file should still
//   keep it a valid obliv-C file after preprocessing. So e.g., no magic
//   conversion from obliv int to OblivBits* inside inline functions.
//   In fact, user code should never be aware of OblivBits type.

// Right now, just input/output functions are here

void setupOblivBool(OblivInputs* spec, obliv bool* dest, bool v);
void setupOblivChar(OblivInputs* spec, obliv char* dest, char v);
void setupOblivInt(OblivInputs* spec, obliv int* dest, int v);
void setupOblivShort(OblivInputs* spec, obliv short* dest, short v);
void setupOblivLong(OblivInputs* spec, obliv long* dest, long v);
void setupOblivLLong(OblivInputs* spec, obliv long long * dest, long long v);

void feedOblivInputs(OblivInputs* spec, size_t count, int party);

bool revealOblivBool(const obliv bool* src,int party);
char revealOblivChar(const obliv char* src,int party);
int revealOblivInt(const obliv int* src,int party);
short revealOblivShort(const obliv short* src,int party);
long revealOblivLong(const obliv long* src,int party);
long long revealOblivLLong(const obliv long long* src,int party);

#endif // OBLIV_H
