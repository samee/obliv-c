#pragma once
#include<gcrypt.h>

#include<obliv_common.h>

#define BC_MAXBLEN 20 // internal constant (i.e. private)
#define BC_SEEDLEN (128/8)

// Simply applies a block cipher in counter mode on zeroes
typedef struct 
{ gcry_cipher_hd_t cipher; unsigned char zeroes[BC_MAXBLEN];
  size_t blen;
} BCipherRandomGen;

// Confusingly, these are implemented in ot.c. 
BCipherRandomGen* newBCipherRandomGen();
void releaseBCipherRandomGen(BCipherRandomGen* gen);
void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len);

// key is assumed to be BC_SEEDLEN bytes long
void resetBCipherRandomGen(BCipherRandomGen* gen,const char* key);
