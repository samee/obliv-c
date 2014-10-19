#include<assert.h>
#include<bcrandom.h>

BCipherRandomGen* newBCipherRandomGen()
{
  const int algo = GCRY_CIPHER_AES128; // change BC_SEEDLEN if this changes
  size_t klen;
  BCipherRandomGen* gen;
  unsigned char key[16];
  int i;
  gcry_cipher_hd_t cipher;

  gcryDefaultLibInit();
  gen = malloc(sizeof(BCipherRandomGen));
  gcry_cipher_open(&cipher,algo,GCRY_CIPHER_MODE_CTR,0);
  gen->cipher = cipher;
  klen = gcry_cipher_get_algo_keylen(algo);
  assert(klen<=BC_SEEDLEN);
  gen->blen = gcry_cipher_get_algo_blklen(algo);
  assert(klen<=sizeof(key));
  assert(gen->blen<=sizeof(gen->zeroes));
  assert(gen->blen>=sizeof(uint64_t)); // used in setctrFromIntBCipherRandomGen
  gcry_randomize(key,klen,GCRY_STRONG_RANDOM);
  gcry_cipher_setkey(cipher,key,klen);
  for(i=0;i<gen->blen;++i) gen->zeroes[i]=gen->ctr[i]=0;
  return gen;
}
void releaseBCipherRandomGen(BCipherRandomGen* gen)
{
  if(gen==NULL) return;
  gcry_cipher_close(gen->cipher);
  free(gen);
}

// key is assumed to be BC_SEEDLEN bytes long
void resetBCipherRandomGen(BCipherRandomGen* gen,const char* key)
{
  gcry_cipher_reset(gen->cipher);
  gcry_cipher_setkey(gen->cipher,key,BC_SEEDLEN);
}

void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len)
{ unsigned char lastout[BC_MAXBLEN];
  int i;
  const size_t blen = gen->blen;
  for(i=0;i+blen<=len;i+=blen)
    gcry_cipher_encrypt(gen->cipher,(unsigned char*)dest+i,blen,
        gen->zeroes,blen);
  if(i<len)
  { gcry_cipher_encrypt(gen->cipher,lastout,blen,gen->zeroes,blen);
    memcpy(dest+i,lastout,len-i); // discard last few bits
  }
}

unsigned long long bcRandomInt(BCipherRandomGen* gen,unsigned long long max)
{
  unsigned long long rv;
  randomizeBuffer(gen,(char*)&rv,sizeof(rv));
  return rv%max; // so about that bias: max should be << 2^64
}

void bcRandomShuffle(BCipherRandomGen* gen,unsigned arr[],size_t n)
{
  size_t i=n;
  while(i-->0)
  { size_t t,j = bcRandomInt(gen,i+1);
    t=arr[j]; arr[j]=arr[i]; arr[i]=t;
  }
}

void bcRandomPermutation(BCipherRandomGen* gen,unsigned arr[],size_t n)
{
  size_t i;
  for(i=0;i<n;++i) arr[i]=i;
  bcRandomShuffle(gen,arr,n);
}

