#include<assert.h>
#include<bcrandom.h>

static const int algo = GCRY_CIPHER_AES128; // change BC_SEEDLEN if this changes

static BCipherRandomGen* newBCipherRandomGenNoKey()
{
  BCipherRandomGen* gen;
  int i;
  gcry_cipher_hd_t cipher;

  gcryDefaultLibInit();
  gen = malloc(sizeof(BCipherRandomGen));
  gcry_cipher_open(&cipher,algo,GCRY_CIPHER_MODE_CTR,0);
  gen->cipher = cipher;
  gen->blen = gcry_cipher_get_algo_blklen(algo);
  assert(gen->blen<=sizeof(gen->zeroes));
  assert(gen->blen>=sizeof(uint64_t)); // used in setctrFromIntBCipherRandomGen
  for(i=0;i<gen->blen;++i) gen->zeroes[i]=gen->ctr[i]=0;
  return gen;
}
BCipherRandomGen* newBCipherRandomGen()
{
  unsigned char key[16];
  BCipherRandomGen* gen = newBCipherRandomGenNoKey();
  size_t klen = gcry_cipher_get_algo_keylen(algo);
  assert(klen<=sizeof(key));
  assert(klen<=BC_SEEDLEN);
  gcry_randomize(key,klen,GCRY_STRONG_RANDOM);
  gcry_cipher_setkey(gen->cipher,key,klen);
  return gen;
}
// Assumes key is BC_SEEDLEN bytes long
BCipherRandomGen* newBCipherRandomGenByKey(const char* key)
{
  BCipherRandomGen* gen = newBCipherRandomGenNoKey();
  size_t klen = gcry_cipher_get_algo_keylen(algo);
  assert(klen<=BC_SEEDLEN);
  gcry_cipher_setkey(gen->cipher,key,klen);
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
void randomizeBufferByKey(const char* key, char* dest,size_t len)
{
  BCipherRandomGen* gen = newBCipherRandomGenByKey(key);
  randomizeBuffer(gen,dest,len);
  releaseBCipherRandomGen(gen);
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

