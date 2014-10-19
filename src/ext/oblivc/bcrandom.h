#pragma once
#include<gcrypt.h>

#include<obliv_common.h>

#define BC_MAXBLEN 20 // internal constant (i.e. private)
#define BC_SEEDLEN (128/8)

// Simply applies a block cipher in counter mode on zeroes
typedef struct 
{ gcry_cipher_hd_t cipher;
  unsigned char zeroes[BC_MAXBLEN], ctr[BC_MAXBLEN];
  size_t blen;
} BCipherRandomGen;

BCipherRandomGen* newBCipherRandomGen();
void releaseBCipherRandomGen(BCipherRandomGen* gen);
// key is assumed to be BC_SEEDLEN bytes long
void resetBCipherRandomGen(BCipherRandomGen* gen,const char* key);

void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len);
gcry_mpi_t dhRandomExp(BCipherRandomGen* gen);

unsigned long long bcRandomInt(BCipherRandomGen* gen,unsigned long long max);
void bcRandomShuffle(BCipherRandomGen* gen,unsigned arr[],size_t n);
void bcRandomPermutation(BCipherRandomGen* gen,unsigned arr[],size_t n);
