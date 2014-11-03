#pragma once
#include<gcrypt.h>

#include<obliv_common.h>

#define BC_MAXBLEN 20 // internal constant (i.e. private)
#define BC_SEEDLEN_DEFAULT (128/8)
#define BC_ALGO_DEFAULT GCRY_CIPHER_AES128
#define BC_SEEDLEN_MAX (256/8)

// Simply applies a block cipher in counter mode on zeroes
typedef struct 
{ gcry_cipher_hd_t cipher;
  unsigned char zeroes[BC_MAXBLEN], ctr[BC_MAXBLEN];
  size_t blen,klen;
  int algo;
} BCipherRandomGen;

BCipherRandomGen* newBCipherRandomGen();
BCipherRandomGen* newBCipherRandomGenByKey(const char* key);
BCipherRandomGen* newBCipherRandomGenByAlgoKey(int algo,const char* key);
BCipherRandomGen* copyBCipherRandomGenNoKey(BCipherRandomGen* bc);
void releaseBCipherRandomGen(BCipherRandomGen* gen);
// key is assumed to be gen->klen long
void resetBCipherRandomGen(BCipherRandomGen* gen,const char* key);

void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len);
// key is assumed to be BC_SEEDLEN_DEFAULT bytes long
void randomizeBufferByKey(const char* key,char* dest,size_t len);
gcry_mpi_t dhRandomExp(BCipherRandomGen* gen);

unsigned long long bcRandomInt(BCipherRandomGen* gen,unsigned long long max);
void bcRandomShuffle(BCipherRandomGen* gen,unsigned arr[],size_t n);
void bcRandomPermutation(BCipherRandomGen* gen,unsigned arr[],size_t n);
