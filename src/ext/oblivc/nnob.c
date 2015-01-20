#include<assert.h>
#include<pthread.h>
#include<obliv_bits.h>
#include<commitReveal.h>
#include<nnob.h>
#include<stdio.h>
#include<time.h>

#define OT_THREAD_THRESHOLD 500
#define OT_THREAD_COUNT 8

void debugTimer(time_struct* t)
{
	clock_gettime(CLOCK_REALTIME, &(t->wallclockTime));
	t->cpuclockTime = clock();
}

void debugPrintTime(time_struct* begin, time_struct* end, char* name, int party)
{
	double wallclockSeconds = (double)((end->wallclockTime.tv_sec+end->wallclockTime.tv_nsec*1e-9) - 
			(double)(begin->wallclockTime.tv_sec+begin->wallclockTime.tv_nsec*1e-9));
	double cpuclockSeconds = (double)(end->cpuclockTime - begin->cpuclockTime)/CLOCKS_PER_SEC;
	fprintf(stderr, "CPU clock time for %s, party %d: %lf\n", name, party, cpuclockSeconds);
	fprintf(stderr, "Wallclock time for %s, party %d: %lf\n", name, party, wallclockSeconds);
}

void debugGetNnobHalfBit(NnobHalfBit* output, bool bit, const nnob_key_t key, 
		const nnob_key_t globalDelta, NnobHalfBitType type)
{
	NnobHalfBit t;
	t.type = type;
	if(type==ShareAndMac)
	{
		t.ShareAndMac.value = bit;
		memcpy(t.ShareAndMac.mac, key, NNOB_KEY_BYTES);
		if(bit) memxor(t.ShareAndMac.mac , globalDelta, NNOB_KEY_BYTES);
	}
	else
	{
		memcpy(t.key, key, NNOB_KEY_BYTES);
	}
	*output=t;
}

void nl()
{
	fprintf(stderr, "\n");
}

void printWithlen(const char* a, int len)
{
	fprintf(stderr, "%.*s", len, a);	
}
void print(const char* a)
{
	fprintf(stderr, "%s", a);	
}

void debugPrintHex(const char* a, int len)
{
	int i;
	for(i=0;i<len;i++) fprintf(stderr, "%02x", a[i]&0xff);
}

void debugPrintXor(const char* a, const char* b, int len)
{
	char* out = malloc(len);
	memcpy(out, a, len);
	memxor(out, b, len);
	debugPrintHex(out,len);
	free(out);
}

void debugPrintXorShowParam(const char* a, const char* b, int len)
{
	debugPrintHex(a,len);
	print(" ^ ");
	debugPrintHex(b,len);
	print(" = ");
	debugPrintXor(a,b,len);
}

void debugPrintNnobHalfBit(const NnobHalfBit* a)
{
	fprintf(stderr,"{type=%s, ", a->type?"ShareAndMac":"Key");
	if(a->type==ShareAndMac)
	{
		fprintf(stderr,"share=%d, mac=", a->ShareAndMac.value);
		debugPrintHex(a->ShareAndMac.mac, NNOB_KEY_BYTES);
	}
	else
	{
		fprintf(stderr,"key=");
		debugPrintHex(a->key, NNOB_KEY_BYTES);
	}
	fprintf(stderr,"}");
}

bool debugMatchingNnobHalfBit(const NnobHalfBit* shareAndMac, const NnobHalfBit* key,
		const nnob_key_t globalDelta)
{
	assert(shareAndMac->type==ShareAndMac);
	assert(key->type==Key);

	bool success=true;
	char temp[NNOB_KEY_BYTES];
	if(shareAndMac->ShareAndMac.value)
	{
		memcpy(temp, shareAndMac->ShareAndMac.mac, NNOB_KEY_BYTES);			
		memxor(temp, globalDelta, NNOB_KEY_BYTES);			
		success&=memcmp(key->key, temp, NNOB_KEY_BYTES)==0?true:false;
	}
	else success&=memcmp(key->key, shareAndMac->ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;
	return success;
}

bool debugMatchingOblivBit(const OblivBit* x, const nnob_key_t globalDeltaToXKey,
		const OblivBit* y, const nnob_key_t globalDeltaToYKey)
{
	bool success=true;
	char temp[NNOB_KEY_BYTES];
	if(x->nnob.share)
	{
		memcpy(temp, x->nnob.mac, NNOB_KEY_BYTES);
		memxor(temp, globalDeltaToYKey, NNOB_KEY_BYTES);
		success&=memcmp(temp, y->nnob.key, NNOB_KEY_BYTES)==0?true:false;
	}
	else success&=memcmp(x->nnob.mac, y->nnob.key, NNOB_KEY_BYTES)==0?true:false;
	if(y->nnob.share)
	{
		memcpy(temp, y->nnob.mac, NNOB_KEY_BYTES);
		memxor(temp, globalDeltaToXKey, NNOB_KEY_BYTES);
		success&=memcmp(temp, x->nnob.key, NNOB_KEY_BYTES)==0?true:false;
		assert(success);
	}
	else success&=memcmp(y->nnob.mac, x->nnob.key, NNOB_KEY_BYTES)==0?true:false;
	return success;
}

void debugPrintKeyfromShareAndMac(const NnobHalfBit* a, const nnob_key_t globalDelta)
{
	assert(a->type==ShareAndMac);
	if(a->ShareAndMac.value) 
		debugPrintXor(a->ShareAndMac.mac, globalDelta, NNOB_KEY_BYTES);
	else debugPrintHex(a->ShareAndMac.mac, NNOB_KEY_BYTES);
}

void debugPrintNnobHalfBitExtra(const NnobHalfBit* a, const nnob_key_t globalDelta)
{
	debugPrintNnobHalfBit(a);
	if(a->type==ShareAndMac)
	{
		print("{associated_key=");
		debugPrintKeyfromShareAndMac(a, globalDelta);	
		print("}");
	}
	else{
		print("{key^globalDelta=");
		debugPrintXor(a->key, globalDelta, NNOB_KEY_BYTES);
		print("}");
	}
}

bool debugCheckOT(const NnobHalfBit* x0_1, const NnobHalfBit* x1_1,
		const NnobHalfBit* c_1, const NnobHalfBit* z_1, const nnob_key_t globalDelta_1,
		const NnobHalfBit* x0_2, const NnobHalfBit* x1_2,
		const NnobHalfBit* c_2, const NnobHalfBit* z_2, const nnob_key_t globalDelta_2)
{
	assert(x0_1->type == x1_1->type);
	assert(x0_1->type != c_1->type);
	assert(c_1->type == z_1->type);
	assert(x0_2->type == x1_2->type);
	assert(x0_2->type != c_2->type);
	assert(c_2->type == z_2->type);
	assert(x0_1->type != x0_2->type);

	bool correct = true;
	if(x0_1->type==ShareAndMac)
	{
		correct&=debugMatchingNnobHalfBit(x0_1, x0_2, globalDelta_2);
		correct&=debugMatchingNnobHalfBit(x1_1, x1_2, globalDelta_2);
		correct&=debugMatchingNnobHalfBit(c_2, c_1, globalDelta_1);
		correct&=debugMatchingNnobHalfBit(z_2, z_1, globalDelta_1);
		correct&=((c_2->ShareAndMac.value?x1_1->ShareAndMac.value:x0_1->ShareAndMac.value)
			==z_2->ShareAndMac.value);
	}
	else
	{
		correct&=debugMatchingNnobHalfBit(x0_2, x0_1, globalDelta_1);
		correct&=debugMatchingNnobHalfBit(x1_2, x1_1, globalDelta_1);
		correct&=debugMatchingNnobHalfBit(c_1, c_2, globalDelta_2);
		correct&=debugMatchingNnobHalfBit(z_1, z_2, globalDelta_2);
		correct&=(c_1->ShareAndMac.value?x1_2->ShareAndMac.value:x0_2->ShareAndMac.value)
			==z_1->ShareAndMac.value;
	}
	assert(correct);
	return correct;
}

bool debugCheckAND(const NnobHalfBit* x_1, const NnobHalfBit* y_1, 
		const NnobHalfBit* z_1, const nnob_key_t globalDelta_1, 
		const NnobHalfBit* x_2, const NnobHalfBit* y_2,
		const NnobHalfBit* z_2, const nnob_key_t globalDelta_2)
{
	assert(x_1->type == y_1->type);
	assert(y_1->type == z_1->type);
	assert(x_2->type == y_2->type);
	assert(y_2->type == z_2->type);
	assert(x_1->type != x_2->type);

	bool correct = true;
	if(x_1->type==ShareAndMac)
	{
		correct&=debugMatchingNnobHalfBit(x_1, x_2, globalDelta_2);
		correct&=debugMatchingNnobHalfBit(y_1, y_2, globalDelta_2);
		correct&=debugMatchingNnobHalfBit(z_1, z_2, globalDelta_2);
		correct&=(z_1->ShareAndMac.value==(x_1->ShareAndMac.value&&y_1->ShareAndMac.value));
	}
	else
	{
		correct&=debugMatchingNnobHalfBit(x_2, x_1, globalDelta_1);
		correct&=debugMatchingNnobHalfBit(y_2, y_1, globalDelta_1);
		correct&=debugMatchingNnobHalfBit(z_2, z_1, globalDelta_1);
		correct&=(z_2->ShareAndMac.value==(x_2->ShareAndMac.value&&y_2->ShareAndMac.value));
	}
	return correct;
}

void debugPrintOblivBit(const OblivBit* bit)
{
	bool b = bit->nnob.share%2==1?true:false;
	fprintf(stderr, "{share=%d, mac=", b);
	debugPrintHex(bit->nnob.mac, NNOB_KEY_BYTES);
	print(", key=");
	debugPrintHex(bit->nnob.key, NNOB_KEY_BYTES);
	print("}");
}

typedef struct
{
  ProtocolDesc* pd;
  int destParty, keyBytes;
  BCipherRandomGen **keyblock;
  bool *S;
  char *spack; // same as S, in packed bytes;
} SenderExtensionBox;

typedef struct
{
  ProtocolDesc* pd;
  int srcParty, keyBytes;
  BCipherRandomGen **keyblock0, **keyblock1;
} RecverExtensionBox;

SenderExtensionBox*
senderExtensionBoxNew (ProtocolDesc* pd, int destParty, int keyBytes);
void
senderExtensionBoxRelease (SenderExtensionBox* s);

RecverExtensionBox*
recverExtensionBoxNew (ProtocolDesc* pd, int srcParty, int keyBytes);
void
recverExtensionBoxRelease (RecverExtensionBox* r);
void
senderExtensionBox(SenderExtensionBox* s,char box[],size_t rowBytes);
void
recverExtensionBox(RecverExtensionBox* r,char box[],
                   const char mask[],size_t rowBytes);

void setctrFromIntBCipherRandomGen(BCipherRandomGen* gen,uint64_t ctr);

static void
unpackBytes(bool* dest, const char* src,int bits)
{
  int i,j;
  for(i=0;i<(bits+7)/8;++i)
  { char ch=src[i];
    for(j=0;j<8 && 8*i+j<bits;++j,ch>>=1) 
      dest[8*i+j]=(ch&1);
  }
}
static int* allRows(int n)
{ int i,*res = malloc(sizeof(int[n]));
  for(i=0;i<n;++i) res[i]=i;
  return res;
}

void
bitmatMul(char* dest,const char* mat,const char* src,int rows,int cols);

typedef struct
{
  char* dest;
  const char *hashmat,*src;
  int rowBytes,from,to;
} BitMatMulThread;


void* bitmatMul_thread_nnob(void* args)
{
  BitMatMulThread* a=args;
  int i;
  for(i=a->from;i<a->to;i++)
    bitmatMul(a->dest+i*NNOB_KEY_BYTES,a->hashmat,a->src+i*a->rowBytes,
              8*NNOB_KEY_BYTES,8*a->rowBytes);
  return NULL;
}

bool
senderExtensionBoxValidate_hhash(SenderExtensionBox* s,BCipherRandomGen* gen,
                                 const char box[],size_t rowBytes);

bool
recverExtensionBoxValidate_hhash(RecverExtensionBox* r,BCipherRandomGen* gen,
                                 const char box[],size_t rowBytes);
bool
senderExtensionBoxValidate_byPair(SenderExtensionBox* s,BCipherRandomGen* gen,
                                  int rowsRemaining[],
                                  const char box[], int rowBytes, bool dohash);
bool
recverExtensionBoxValidate_byPair(RecverExtensionBox* r,BCipherRandomGen* gen,
                                  int rowsRemaining[],
                                  const char box[],const char mask[],
                                  int rowBytes, bool dohash);
// Computes ceil(log_b(x))
static int logfloor(int x,int b)
{ int res=0;
  while(b<=x) { x/=b; res++; }
  return res;
}

static void
bcipherCrypt(BCipherRandomGen* gen,const char* key,int klen,int nonce,
                  char* dest,const char* src,int n)
{
  int i;
  char keyx[gen->klen];
  assert(klen<=gen->klen);
  memcpy(keyx,key,klen); memset(keyx+klen,0,gen->klen-klen);
  resetBCipherRandomGen(gen,keyx);
  setctrFromIntBCipherRandomGen(gen,nonce);
  randomizeBuffer(gen,dest,n);
  for(i=0;i<n;++i) dest[i]^=src[i];
}

void nnobHash(BCipherRandomGen* gen, const char* key, int klen, int nonce, 
		char dest[NNOB_KEY_BYTES], const char* src)
{
	bcipherCrypt(gen, key, klen, nonce, dest, src, NNOB_KEY_BYTES);	
}

void (*nnobPRG)(BCipherRandomGen* gen,const char* key,int klen,int nonce,
                  char* dest,const char* src,int n) = bcipherCrypt;

NnobProtocolDesc* initNnobProtocolDesc(ProtocolDesc* pd, int numOTs, OTExtValidation validation,
		int destparty)
{
	/*int denom = logfloor(numOTs, 2)+1;*/
	/*int bucketSize = (int)(1 + (NNOB_KEY_BYTES*8)/denom);*/
	int bucketSize = BUCKET_SIZE;
	int allOTs = (OTS_PER_aOT+OTS_PER_aAND)*bucketSize+OTS_FOR_R+OTS_FOR_INPUTS;

	NnobProtocolDesc* npd = malloc(sizeof(NnobProtocolDesc));
	pd->extra=npd;
	npd->bucketSize = bucketSize;
	int n = ((numOTs+7)/8)*8;
	int aOTs = n/allOTs;
	int aANDs = n/allOTs;
	assert(aOTs!=0);
	assert(aANDs!=0);

	npd->aBitsShareAndMac.counter = 0;
	npd->aBitsShareAndMac.n = n;
	npd->aBitsShareAndMac.share = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsShareAndMac.mac = malloc(n*NNOB_KEY_BYTES);

	npd->aBitsKey.key = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsKey.counter = 0;
	npd->aBitsKey.n = n;

	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x0 = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x1 = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.c = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.z = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.counter = 0;
	npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.n = aOTs;

	npd->FDeal.aOTQuadruple.aOTKeyOfZ.x0 = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTKeyOfZ.x1 = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTKeyOfZ.c = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTKeyOfZ.z = malloc(aOTs*sizeof(NnobHalfBit));
	npd->FDeal.aOTQuadruple.aOTKeyOfZ.counter = 0;
	npd->FDeal.aOTQuadruple.aOTKeyOfZ.n = aOTs;

	npd->FDeal.aANDTriple.aANDShareAndMac.x = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDShareAndMac.y = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDShareAndMac.z = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDShareAndMac.counter = 0;
	npd->FDeal.aANDTriple.aANDShareAndMac.n = aANDs;

	npd->FDeal.aANDTriple.aANDKey.x = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDKey.y = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDKey.z = malloc(aANDs*sizeof(NnobHalfBit));
	npd->FDeal.aANDTriple.aANDKey.counter = 0;
	npd->FDeal.aANDTriple.aANDKey.n = aANDs;

	npd->error = false;

	time_struct begin, end;
	char mat1[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char mat2[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char* aBitFullMac = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	char* aBitFullKey = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	if(destparty==1)
	{
		debugTimer(&begin);
		npd->error |= WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat1, aBitFullMac, numOTs, validation, destparty);
		npd->error |= WaBitBoxGetKey(pd, npd->globalDelta, 
				mat2, aBitFullKey, numOTs, validation, destparty);
		debugTimer(&end);
		debugPrintTime(&begin, &end, "WaBitBox", 1);
		debugTimer(&begin);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat1, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat2, numOTs);
		debugTimer(&end);
		debugPrintTime(&begin, &end, "aBitBox", 1);
		/*npd->error |= aBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.mac, */
				/*npd->aBitsShareAndMac.share, numOTs, validation, destparty);*/
		/*npd->error |= aBitBoxGetKey(pd, npd->aBitsKey.key, npd->globalDelta, */
				/*numOTs, validation, destparty);*/
		debugTimer(&begin);
		npd->error |= aOT(pd, npd, Key, bucketSize, destparty);
		npd->error |= aOT(pd, npd, ShareAndMac, bucketSize, destparty);
		debugTimer(&end);
		debugPrintTime(&begin, &end, "aOT", 1);
		debugTimer(&begin);
		npd->error |= aAND(pd, npd, ShareAndMac, bucketSize, destparty);
		npd->error |= aAND(pd, npd, Key, bucketSize, destparty);
		debugTimer(&end);
		debugPrintTime(&begin, &end, "aAND", 1);
	}
	else
	{
		npd->error |= WaBitBoxGetKey(pd, npd->globalDelta, 
				mat1, aBitFullKey, numOTs, validation, destparty);
		npd->error |= WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat2, aBitFullMac, numOTs, validation, destparty);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat2, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat1, numOTs);
		/*npd->error |= aBitBoxGetKey(pd, npd->aBitsKey.key, npd->globalDelta, */
				/*numOTs, validation, destparty);*/
		/*npd->error |= aBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.mac, */
				/*npd->aBitsShareAndMac.share, numOTs, validation, destparty);*/
		npd->error |= aOT(pd, npd, ShareAndMac, bucketSize, destparty);
		npd->error |= aOT(pd, npd, Key, bucketSize, destparty);
		npd->error |= aAND(pd, npd, Key, bucketSize, destparty);
		npd->error |= aAND(pd, npd, ShareAndMac, bucketSize, destparty);
	}
	free(aBitFullKey);
	free(aBitFullMac);
	return npd;
}

void cleanupNnobProtocol(NnobProtocolDesc* npd)
{
	free(npd->aBitsShareAndMac.share);
	free(npd->aBitsShareAndMac.mac); 
	free(npd->aBitsKey.key); 
	free(npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x0); 
	free(npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x1); 
	free(npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.c); 
	free(npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.z); 
	free(npd->FDeal.aOTQuadruple.aOTKeyOfZ.x0);
	free(npd->FDeal.aOTQuadruple.aOTKeyOfZ.x1);
	free(npd->FDeal.aOTQuadruple.aOTKeyOfZ.c);
	free(npd->FDeal.aOTQuadruple.aOTKeyOfZ.z);
	free(npd->FDeal.aANDTriple.aANDShareAndMac.x);
	free(npd->FDeal.aANDTriple.aANDShareAndMac.y);
	free(npd->FDeal.aANDTriple.aANDShareAndMac.z);
	free(npd->FDeal.aANDTriple.aANDKey.x);
	free(npd->FDeal.aANDTriple.aANDKey.y);
	free(npd->FDeal.aANDTriple.aANDKey.z);
	free(npd);
}

typedef struct TransposeThread {
	int* rows;
	char* src;
	char* dest;
	int rc;
	int rowBytes;
	int from;
	int to;
} TransposeThread;

void* transpose_thread(void* args){
	TransposeThread* a=args;
	int i,j;
	char* boxColumn;
	for(i=a->from;i<a->to;i++) {
		for(j=0;j<a->rc;j++) {
			boxColumn=a->src+a->rows[j]*a->rowBytes;
			setBit(a->dest+i*A_BIT_PARAMETER_BYTES, j, getBit(boxColumn,i));
		}
	}
	/*for(i=0;i<n;i++) // go through columns*/
	/*{*/
		/*for(j=0;j<rc;j++) // go through rows*/
		/*{*/
			/*boxColumn=box+rows[j]*rowBytes;*/
			/*setBit(aBitFullMac+i*A_BIT_PARAMETER_BYTES, j, getBit(boxColumn, i));*/
		/*}*/
	/*}*/
	return NULL;
}
bool WaBitBoxGetBitAndMac(ProtocolDesc* pd, bool* b,
		char* mat, char* aBitFullMac,
		int n, OTExtValidation validation, int destparty){
	assert(n%8==0);
	int i,j;
	bool success = true;
	int k = 8*A_BIT_PARAMETER_BYTES;
	k*=validation==OTExtValidation_hhash?1:2;
	int rowBytes = (n+7)/8;
	int *rows,rc;
	char *box = malloc(k*rowBytes);
	char *mask = malloc(rowBytes);
	BCipherRandomGen* gen = newBCipherRandomGen();
	RecverExtensionBox* r =recverExtensionBoxNew(pd, destparty, k/8);
	randomizeBuffer(gen,mask,rowBytes);
	unpackBytes(b,mask,n);
	recverExtensionBox(r,box,mask,rowBytes);
	if(validation==OTExtValidation_hhash)
	{ 
		rows = allRows(k);
		rc=k;
		if(!recverExtensionBoxValidate_hhash(r, gen, box, rowBytes))
			success = false;
	}
	else
	{ 
		rows = malloc(sizeof(int[k/2]));
		rc=k/2;
		if(!recverExtensionBoxValidate_byPair(r, gen, rows,
					box,mask,rowBytes, true))
			success = false;
	}
	if(!success) r->pd->error = OC_ERROR_OT_EXTENSION;
	success&=ocRandomBytes(pd, gen, mat,8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES, destparty);
	assert(rc==8*A_BIT_PARAMETER_BYTES);
	int tc=OT_THREAD_COUNT, done=0;
	TransposeThread args[OT_THREAD_COUNT];
	pthread_t transpt[OT_THREAD_COUNT];
	for(i=0;i<tc;++i)
	{
		int c = (n-done)/(tc-i); 
		args[i] = (TransposeThread){.dest=aBitFullMac,.src=box,
			.rc=rc,.rows=rows,.rowBytes=rowBytes,.from=done,.to=done+c};
		if(i==tc-1) transpose_thread(&args[i]); // no thread if tc==1
		else pthread_create(&transpt[i],NULL,transpose_thread,&args[i]);
		done+=c;
	}
	for(i=0;i<tc-1;++i) pthread_join(transpt[i],NULL);
	/*for(i=0;i<n;i++) // go through columns*/
	/*{*/
		/*for(j=0;j<rc;j++) // go through rows*/
		/*{*/
			/*boxColumn=box+rows[j]*rowBytes;*/
			/*setBit(aBitFullMac+i*A_BIT_PARAMETER_BYTES, j, getBit(boxColumn, i));*/
		/*}*/
	/*}*/

	free(mask);
	free(rows);
	free(box);
	recverExtensionBoxRelease(r);
	releaseBCipherRandomGen(gen);
	return success;
}

bool WaBitBoxGetKey(ProtocolDesc* pd, nnob_key_t globalDelta,
		char* mat, char* aBitFullKey,
		int n, OTExtValidation validation, int destparty){
	assert(n%8==0);
	int i,j;
	bool success = true;
	int k = 8*A_BIT_PARAMETER_BYTES;
	k*=validation==OTExtValidation_hhash?1:2;
	int rowBytes = (n+7)/8;
	int *rows, rc;
	char *box = malloc(k*rowBytes);
	BCipherRandomGen* gen = newBCipherRandomGen();
	SenderExtensionBox* s = senderExtensionBoxNew(pd, destparty, k/8);
	senderExtensionBox(s,box,rowBytes);
	if(validation==OTExtValidation_hhash)
	{ 
		rows = allRows(k);
		rc = k;
		if(!senderExtensionBoxValidate_hhash(s,gen,box,rowBytes))
			success = false;
	} else
	{ rows = malloc(sizeof(int[k/2]));
		rc = k/2;
		if(!senderExtensionBoxValidate_byPair(s,gen,rows,box,rowBytes, true))
			success = false;
		for(i=0;i<rc;++i) setBit(s->spack,i,s->S[rows[i]]);
	}
	if(!success) s->pd->error = OC_ERROR_OT_EXTENSION;
	success&=ocRandomBytes(pd, gen, mat,8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES, destparty);
	bitmatMul(globalDelta, mat, s->spack, 8*NNOB_KEY_BYTES, rc);
	assert(rc==8*A_BIT_PARAMETER_BYTES);
	int tc=OT_THREAD_COUNT, done=0;
	TransposeThread args[OT_THREAD_COUNT];
	pthread_t transpt[OT_THREAD_COUNT];
	for(i=0;i<tc;++i)
	{
		int c = (n-done)/(tc-i); 
		args[i] = (TransposeThread){.dest=aBitFullKey,.src=box,
			.rc=rc,.rows=rows,.rowBytes=rowBytes,.from=done,.to=done+c};
		if(i==tc-1) transpose_thread(&args[i]); // no thread if tc==1
		else pthread_create(&transpt[i],NULL,transpose_thread,&args[i]);
		done+=c;
	}
	for(i=0;i<tc-1;++i) pthread_join(transpt[i],NULL);
	/*for(i=0;i<n;i++) // go through columns*/
	/*{*/
		/*for(j=0;j<rc;j++) // go through rows*/
		/*{*/
			/*boxColumn=box+rows[j]*rowBytes;*/
			/*setBit(aBitFullKey+i*A_BIT_PARAMETER_BYTES, j, getBit(boxColumn, i));*/
		/*}*/
	/*}*/

	free(rows);
	free(box);
	senderExtensionBoxRelease(s);
	releaseBCipherRandomGen(gen);
	return success;
}

void WaBitToaBit(char* aBit, char* WaBit, char* mat, int n){
	int rowBytes = (n+7)/8, i,done=0,tc;
	BitMatMulThread args[OT_THREAD_COUNT];
	pthread_t hasht[OT_THREAD_COUNT];
	if(8*rowBytes<=OT_THREAD_THRESHOLD) tc=1;
	else tc=OT_THREAD_COUNT;

	for(i=0;i<tc;++i)
	{
		int c = (n-done)/(tc-i); 
		args[i] = (BitMatMulThread){.dest=aBit,.hashmat=mat,.src=WaBit,
			.rowBytes=A_BIT_PARAMETER_BYTES,.from=done,.to=done+c};
		if(i==tc-1) bitmatMul_thread_nnob(&args[i]); // no thread if tc==1
		else pthread_create(&hasht[i],NULL,bitmatMul_thread_nnob,&args[i]);
		done+=c;
	}
	for(i=0;i<tc-1;++i) pthread_join(hasht[i],NULL);
}

void randomOblivAuthentication(NnobProtocolDesc* npd,
		NnobHalfBit* bit, NnobHalfBitType type)
{
	int counter;
	bit->type = type;
	if(type==ShareAndMac){
		counter = npd->aBitsShareAndMac.counter;
		npd->aBitsShareAndMac.counter+=1;
		assert(counter<npd->aBitsShareAndMac.n);
		bit->ShareAndMac.value = npd->aBitsShareAndMac.share[counter];
		memcpy(bit->ShareAndMac.mac, npd->aBitsShareAndMac.mac+counter*NNOB_KEY_BYTES, NNOB_KEY_BYTES);
	}
	else
	{
		counter = npd->aBitsKey.counter;
		npd->aBitsKey.counter+=1;
		assert(counter<npd->aBitsKey.n);
		memcpy(bit->key, npd->aBitsKey.key+counter*NNOB_KEY_BYTES, NNOB_KEY_BYTES);
	}
}


void nnobSetBitXor(ProtocolDesc* pd, NnobHalfBit* dest, const NnobHalfBit* src)
{
	assert(src->type==dest->type);
	if(dest->type==ShareAndMac)
	{
		dest->ShareAndMac.value ^= src->ShareAndMac.value;
		memxor(dest->ShareAndMac.mac, src->ShareAndMac.mac, NNOB_KEY_BYTES);
	}
	else
	{
		memxor(dest->key, src->key, NNOB_KEY_BYTES);
	}
}

void nnobSetBitXorConst(ProtocolDesc* pd, NnobHalfBit* output, bool src)
{
	if(!src) return;
	if(output->type==ShareAndMac)
	{
		output->ShareAndMac.value ^= src;
	}
	else
	{
		NnobProtocolDesc* npd = pd->extra;
		assert(npd!=NULL);
		char* globalDelta = npd->globalDelta;
		memxor(output->key, globalDelta, NNOB_KEY_BYTES);
	}
}

bool nnobSetBitAndGetXY(ProtocolDesc* pd, NnobProtocolDesc* npd, 
		NnobHalfBit* xy, const NnobHalfBit* x, const NnobHalfBit* y, int destparty)
{
	assert(x->type==y->type);
	/*print("x in xy: ");*/
	/*debugPrintNnobHalfBit(x);*/
	/*nl();*/

	/*print("y in xy: ");*/
	/*debugPrintNnobHalfBit(y);*/
	/*nl();*/
	bool success = true;
	
	NnobHalfBit u;
	NnobHalfBit v;
	NnobHalfBit w;
	NnobHalfBit f;
	NnobHalfBit g;

	NnobHalfBitType type = x->type;

	getRandomAANDTriple(npd, &u, &v, &w, type);
	if(type==ShareAndMac)
	{
		f=u;
		nnobSetBitXor(pd, &f, x);
		g=v;
		nnobSetBitXor(pd, &g, y);
		osend(pd, destparty, &f, sizeof(NnobHalfBit));	
		osend(pd, destparty, &g, sizeof(NnobHalfBit));	
	}
	else
	{
		nnob_key_t fMac;
		nnob_key_t gMac;

		orecv(pd, destparty, &f, sizeof(NnobHalfBit));	
		orecv(pd, destparty, &g, sizeof(NnobHalfBit));	
		assert(u.type==Key);
		memcpy(fMac, u.key, NNOB_KEY_BYTES);
		memxor(fMac, x->key, NNOB_KEY_BYTES);
		if(f.ShareAndMac.value) memxor(fMac, npd->globalDelta, NNOB_KEY_BYTES); 
		success&=memcmp(fMac, f.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;


		assert(v.type==Key);
		memcpy(gMac, v.key, NNOB_KEY_BYTES);
		memxor(gMac, y->key, NNOB_KEY_BYTES);
		if(g.ShareAndMac.value) memxor(gMac, npd->globalDelta, NNOB_KEY_BYTES); 
		success&=memcmp(gMac, g.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;
	}

	*xy = w;
	if(f.ShareAndMac.value) nnobSetBitXor(pd, xy, y);
	if(g.ShareAndMac.value) nnobSetBitXor(pd, xy, x);
	if(f.ShareAndMac.value&&g.ShareAndMac.value) 
		nnobSetBitXorConst(pd, xy, true);


	/*print("xy in xy: ");*/
	/*if(xy->type==Key)*/
	/*{*/
		/*debugPrintNnobHalfBitExtra(xy, npd->globalDelta);*/
	/*}*/
	/*else*/
	/*{*/
		/*debugPrintNnobHalfBit(xy);*/
	/*}	*/
	/*nl();*/
	return success;
}

bool nnobSetBitAndGetS(ProtocolDesc* pd, NnobProtocolDesc* npd,
		NnobHalfBit* sYours, const NnobHalfBit* r, const NnobHalfBit* x, 
		const NnobHalfBit* y, int destparty)
{
	assert(x->type!=y->type);

	/*print("x in s: ");*/
	/*debugPrintNnobHalfBit(x);*/
	/*nl();*/

	/*print("y in s: ");*/
	/*debugPrintNnobHalfBit(y);*/
	/*nl();*/
	/*print("r in s: ");*/
	/*debugPrintNnobHalfBit(r);*/
	/*nl();*/
	NnobHalfBit u0;	
	NnobHalfBit u1;	
	NnobHalfBit c;	
	NnobHalfBit w;	
	NnobHalfBit d;
	NnobHalfBit f;
	NnobHalfBit g;
	bool success = true;

	NnobHalfBitType type = x->type;

	getRandomAOTQuadruple(npd, &u0, &u1, &c, &w, !type);
	/*print("u0 in s: ");*/
	/*debugPrintNnobHalfBit(&u0);*/
	/*nl();*/
	/*print("u1 in s: ");*/
	/*debugPrintNnobHalfBit(&u1);*/
	/*nl();*/
	/*print("c in s: ");*/
	/*debugPrintNnobHalfBit(&c);*/
	/*nl();*/
	/*print("w in s: ");*/
	/*debugPrintNnobHalfBit(&w);*/
	/*nl();*/

	if(type==ShareAndMac)
	{
		nnob_key_t dMac;
		orecv(pd, destparty, &d, sizeof(NnobHalfBit));
		assert(c.type==Key);
		memcpy(dMac, c.key, NNOB_KEY_BYTES);
		memxor(dMac, y->key, NNOB_KEY_BYTES);
		if(d.ShareAndMac.value) memxor(dMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(dMac, d.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;
		assert(success);

		f=u0;
		nnobSetBitXor(pd, &f, &u1);
		nnobSetBitXor(pd, &f, x);
		g=u0;
		nnobSetBitXor(pd, &g, r);
		if(d.ShareAndMac.value) nnobSetBitXor(pd, &g, x);
		osend(pd, destparty, &f, sizeof(NnobHalfBit));
		osend(pd, destparty, &g, sizeof(NnobHalfBit));
	}
	else
	{
		nnob_key_t fMac;
		nnob_key_t gMac;
		d=c;
		nnobSetBitXor(pd, &d, y);
		/*fprintf(stderr, "d = c^y = %d\n", d);*/
		osend(pd, destparty, &d, sizeof(NnobHalfBit));

		orecv(pd, destparty, &f, sizeof(NnobHalfBit));
		orecv(pd, destparty, &g, sizeof(NnobHalfBit));
		assert(u0.type==Key);
		assert(u1.type==Key);
		memcpy(fMac, u0.key, NNOB_KEY_BYTES);
		memxor(fMac, u1.key, NNOB_KEY_BYTES);
		memxor(fMac, x->key, NNOB_KEY_BYTES);
		if(f.ShareAndMac.value) memxor(fMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(fMac, f.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;

		assert(r->type==Key);
		memcpy(gMac, r->key, NNOB_KEY_BYTES);
		memxor(gMac, u0.key, NNOB_KEY_BYTES);
		if(d.ShareAndMac.value) memxor(gMac, x->key, NNOB_KEY_BYTES);
		if(g.ShareAndMac.value) memxor(gMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(gMac, g.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;
		assert(success);
	}
	*sYours=w;
	if(f.ShareAndMac.value) nnobSetBitXor(pd, sYours, &c);
	nnobSetBitXorConst(pd, sYours, g.ShareAndMac.value);

	/*print("sYours in s: ");*/
	/*if(sYours->type==Key)*/
	/*{*/
		/*debugPrintNnobHalfBitExtra(sYours, npd->globalDelta);*/
	/*}*/
	/*else*/
	/*{*/
		/*debugPrintNnobHalfBit(sYours);*/
	/*}	*/
	/*nl();*/
	return success;
}

bool nnobSetBitAnd(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit *y)
{

	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);

	bool success=true;

	NnobHalfBit xMac;
	xMac.type = ShareAndMac;
	xMac.ShareAndMac.value =  x->nnob.share;
	memcpy(xMac.ShareAndMac.mac, x->nnob.mac, NNOB_KEY_BYTES);

	NnobHalfBit xKey;
	xKey.type = Key;
	memcpy(xKey.key, x->nnob.key, NNOB_KEY_BYTES);
	/*print("xKey: ");*/
	/*debugPrintNnobHalfBit(&xKey);*/
	/*nl();*/

	NnobHalfBit yMac;
	yMac.type = ShareAndMac;
	yMac.ShareAndMac.value =  y->nnob.share;
	memcpy(yMac.ShareAndMac.mac, y->nnob.mac, NNOB_KEY_BYTES);
	/*print("yMac: ");*/
	/*debugPrintNnobHalfBit(&yMac);*/
	/*nl();*/

	NnobHalfBit yKey;
	yKey.type = Key;
	memcpy(yKey.key, y->nnob.key, NNOB_KEY_BYTES);

	NnobHalfBit zMac;	
	NnobHalfBit zKey;	
	NnobHalfBit rMac;	
	NnobHalfBit rKey;	
	NnobHalfBit xyMac;	
	NnobHalfBit xyKey;	
	NnobHalfBit sMac;	
	NnobHalfBit sKey;	

	randomOblivAuthentication(npd, &rMac, ShareAndMac);
	randomOblivAuthentication(npd, &rKey, Key);

	int destparty = (pd->thisParty*2)%3;

	if(pd->thisParty==1)
	{
		success&=nnobSetBitAndGetXY(pd, npd, &xyMac, &xMac, &yMac, destparty);
		success&=nnobSetBitAndGetS(pd, npd, &sKey, &rMac, &xMac, &yKey, destparty);

		success&=nnobSetBitAndGetXY(pd, npd, &xyKey, &xKey, &yKey, destparty);
		success&=nnobSetBitAndGetS(pd, npd, &sMac, &rKey, &xKey, &yMac, destparty);
	}
	else
	{
		success&=nnobSetBitAndGetXY(pd, npd, &xyKey, &xKey, &yKey, destparty);
		success&=nnobSetBitAndGetS(pd, npd, &sMac, &rKey, &xKey, &yMac, destparty);

		success&=nnobSetBitAndGetXY(pd, npd, &xyMac, &xMac, &yMac, destparty);
		success&=nnobSetBitAndGetS(pd, npd, &sKey, &rMac, &xMac, &yKey, destparty);

	}

	zMac=rMac;
	nnobSetBitXor(pd, &zMac, &sMac);
	nnobSetBitXor(pd, &zMac, &xyMac);

	zKey=rKey;
	nnobSetBitXor(pd, &zKey, &sKey);
	nnobSetBitXor(pd, &zKey, &xyKey);

	z->nnob.share = zMac.ShareAndMac.value;
	memcpy(z->nnob.mac,zMac.ShareAndMac.mac, NNOB_KEY_BYTES);
	memcpy(z->nnob.key,zKey.key, NNOB_KEY_BYTES);
	return success;
}

void nnobHalfBittoOblivBit(OblivBit* obit, NnobHalfBit* hbitMac, NnobHalfBit* hbitKey)
{
	assert(hbitMac->type==ShareAndMac);
	assert(hbitKey->type==Key);
	obit->nnob.share = hbitMac->ShareAndMac.value;
	memcpy(obit->nnob.mac, hbitMac->ShareAndMac.mac, NNOB_KEY_BYTES);
	memcpy(obit->nnob.key, hbitKey->key, NNOB_KEY_BYTES);
}

void nnobGetOblivInputSendShare(ProtocolDesc* pd, NnobProtocolDesc* npd, bool input,
		OblivBit* myInput)
{
	NnobHalfBit myInputKey;
	NnobHalfBit myInputMac;
	bool b;
	int destparty = (pd->thisParty*3)%2;
	randomOblivAuthentication(npd, &myInputMac, ShareAndMac);	
	b = input ^ myInputMac.ShareAndMac.value;
	osend(pd, destparty, &b, 1);
	myInputKey.type = Key;
	memset(myInputKey.key, 0, NNOB_KEY_BYTES);
	nnobSetBitXorConst(pd, &myInputKey, b);
	nnobHalfBittoOblivBit(myInput, &myInputMac, &myInputKey);
}

void nnobGetOblivInputRecvShare(ProtocolDesc* pd, NnobProtocolDesc* npd, OblivBit* yourInput)
{
	NnobHalfBit yourInputKey;
	NnobHalfBit yourInputMac;
	bool b;
	int destparty = (pd->thisParty*3)%2;
	randomOblivAuthentication(npd, &yourInputKey, Key);	
	orecv(pd, destparty, &b, 1);
	yourInputMac.type = ShareAndMac;
	yourInputMac.ShareAndMac.value = b;
	memset(yourInputMac.ShareAndMac.mac, 0, NNOB_KEY_BYTES);
	nnobHalfBittoOblivBit(yourInput, &yourInputMac, &yourInputKey);
}

void nnobGetOblivInput(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* input,
		OblivBit* x, OblivBit* y, int numOblivInput)
{
	int i;
	int destparty = (pd->thisParty*3)%2;

	NnobHalfBit* yourInputKey = malloc(numOblivInput*sizeof(NnobHalfBit));
	NnobHalfBit* yourInputMac = malloc(numOblivInput*sizeof(NnobHalfBit));
	
	NnobHalfBit* myInputKey = malloc(numOblivInput*sizeof(NnobHalfBit));
	NnobHalfBit* myInputMac = malloc(numOblivInput*sizeof(NnobHalfBit));

	bool* b = malloc(numOblivInput);

	for(i=0;i<numOblivInput;i++)
	{
		if(pd->thisParty==1)
		{
			randomOblivAuthentication(npd, &myInputMac[i], ShareAndMac);	
			b[i] = input[i] ^ myInputMac[i].ShareAndMac.value;
			myInputKey[i].type = Key;
			memset(myInputKey[i].key, 0, NNOB_KEY_BYTES);
			nnobSetBitXorConst(pd, &myInputKey[i], b[i]);
			nnobHalfBittoOblivBit(&x[i], &myInputMac[i], &myInputKey[i]);
		}
		else
		{
			randomOblivAuthentication(npd, &yourInputKey[i], Key);	
		}
	}
	if(pd->thisParty==1)
	{
		osend(pd, destparty, b, numOblivInput);
	}
	else
	{
		orecv(pd, destparty, b, numOblivInput);
	}
	for(i=0;i<numOblivInput;i++)
	{
		if(pd->thisParty!=1)
		{
			yourInputMac[i].type = ShareAndMac;
			yourInputMac[i].ShareAndMac.value = b[i];
			memset(yourInputMac[i].ShareAndMac.mac, 0, NNOB_KEY_BYTES);
			nnobHalfBittoOblivBit(&x[i], &yourInputMac[i], &yourInputKey[i]);
		}
	}
	
	
	for(i=0;i<numOblivInput;i++)
	{
		if(pd->thisParty==1)
		{
			randomOblivAuthentication(npd, &yourInputKey[i], Key);	
		}
		else
		{
			randomOblivAuthentication(npd, &myInputMac[i], ShareAndMac);	
			b[i] = input[i] ^ myInputMac[i].ShareAndMac.value;
			myInputKey[i].type = Key;
			memset(myInputKey[i].key, 0, NNOB_KEY_BYTES);
			nnobSetBitXorConst(pd, &myInputKey[i], b[i]);
			nnobHalfBittoOblivBit(&y[i], &myInputMac[i], &myInputKey[i]);
		}
	}
	if(pd->thisParty==1)
	{
		orecv(pd, destparty, b, numOblivInput);
	}
	else
	{
		osend(pd, destparty, b, numOblivInput);
	}
	for(i=0;i<numOblivInput;i++)
	{
		if(pd->thisParty==1)
		{
			yourInputMac[i].type = ShareAndMac;
			yourInputMac[i].ShareAndMac.value = b[i];
			memset(yourInputMac[i].ShareAndMac.mac, 0, NNOB_KEY_BYTES);
			nnobHalfBittoOblivBit(&y[i], &yourInputMac[i], &yourInputKey[i]);
		}
	}

	free(yourInputKey);
	free(yourInputMac);
	free(myInputKey);
	free(myInputMac);
	free(b);

	/*if(pd->thisParty==1)*/
	/*{*/
		/*nnobGetOblivInputSendShare(pd, npd, input, x);*/
		/*nnobGetOblivInputRecvShare(pd, npd, y);*/
	/*}*/
	/*else*/
	/*{*/
		/*nnobGetOblivInputRecvShare(pd, npd, x);*/
		/*nnobGetOblivInputSendShare(pd, npd, input, y);*/
	/*}*/
}

bool nnobRevealOblivBit(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* output, 
		OblivBit* input)
{
	bool success = true;
	nnob_key_t inputMac;
	bool inputBit;
	if(pd->thisParty==1)
	{
		osend(pd, 2, input->nnob.mac, NNOB_KEY_BYTES);
		osend(pd, 2, &input->nnob.share, 1);

		orecv(pd, 2, inputMac, NNOB_KEY_BYTES);
		orecv(pd, 2, &inputBit, 1);
		if(inputBit) memxor(inputMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(inputMac, input->nnob.key,NNOB_KEY_BYTES)==0?true:false;
	}
	else
	{
		orecv(pd, 1, inputMac, NNOB_KEY_BYTES);
		orecv(pd, 1, &inputBit, 1);
		if(inputBit) memxor(inputMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(inputMac, input->nnob.key,NNOB_KEY_BYTES)==0?true:false;

		osend(pd, 1, input->nnob.mac, NNOB_KEY_BYTES);
		osend(pd, 1, &input->nnob.share, 1);
	}
	*output = inputBit^input->nnob.share;
	assert(success);
	return success;
}

bool LaOT(ProtocolDesc* pd, NnobProtocolDesc* npd, const NnobHalfBit* x0, 
		const NnobHalfBit* x1, const NnobHalfBit* c,
		const NnobHalfBit* r, NnobHalfBit* z,  BCipherRandomGen* padder, 
		BCipherRandomGen* gen, int* nonce, int numLaOTs, int destparty)  
{
	assert(x0[0].type==x1[0].type);
	assert(c[0].type==c[0].type);
	assert(c[0].type!=x0[0].type);
	
	int i;
	bool success = true;
	const int XOTBytes = 1+2*NNOB_KEY_BYTES;
	char (*X0)[XOTBytes] = malloc(numLaOTs*XOTBytes);
	char (*X1)[XOTBytes] = malloc(numLaOTs*XOTBytes);
	char (*I0)[NNOB_KEY_BYTES] = malloc(numLaOTs*NNOB_KEY_BYTES);
	char (*I1)[NNOB_KEY_BYTES] = malloc(numLaOTs*NNOB_KEY_BYTES);
	char (*T0)[NNOB_KEY_BYTES] = malloc(numLaOTs*NNOB_KEY_BYTES);
	char (*T1)[NNOB_KEY_BYTES] = malloc(numLaOTs*NNOB_KEY_BYTES);
	char temp[NNOB_KEY_BYTES];
	char (*mc)[XOTBytes] = malloc(numLaOTs*XOTBytes); 
	bool* d = malloc(numLaOTs);
	const char* globalDelta = npd->globalDelta;
	

	for(i=0;i<numLaOTs;i++)
	{
		if(x0[0].type==ShareAndMac)
		{
			char m0[XOTBytes], m1[XOTBytes];
			randomizeBuffer(gen, T0[i], NNOB_KEY_BYTES);
			randomizeBuffer(gen, T1[i], NNOB_KEY_BYTES);

			m0[0] = x0[i].ShareAndMac.value;
			memcpy(m0+1, x0[i].ShareAndMac.mac, NNOB_KEY_BYTES);
			if(x0[i].ShareAndMac.value) memcpy(m0+1+NNOB_KEY_BYTES, T1[i], NNOB_KEY_BYTES);
			else memcpy(m0+1+NNOB_KEY_BYTES, T0[i], NNOB_KEY_BYTES);

			m1[0] = x1[i].ShareAndMac.value;
			memcpy(m1+1, x1[i].ShareAndMac.mac, NNOB_KEY_BYTES);
			if(x1[i].ShareAndMac.value) memcpy(m1+1+NNOB_KEY_BYTES, T1[i], NNOB_KEY_BYTES);
			else memcpy(m1+1+NNOB_KEY_BYTES, T0[i], NNOB_KEY_BYTES);

			nnobPRG(padder, c[i].key, NNOB_KEY_BYTES, *nonce, X0[i], m0, XOTBytes);	
			memcpy(temp, c[i].key, NNOB_KEY_BYTES);
			memxor(temp, globalDelta, NNOB_KEY_BYTES);
			nnobPRG(padder, temp, NNOB_KEY_BYTES, *nonce, X1[i], m1, XOTBytes);	
			*nonce+=1;
		}	
	}
	if(x0[0].type==ShareAndMac)
	{
		osend(pd, destparty, X0, numLaOTs*XOTBytes);
		osend(pd, destparty, X1, numLaOTs*XOTBytes);
	}
	else
	{
		orecv(pd, destparty, X0, numLaOTs*XOTBytes);
		orecv(pd, destparty, X1, numLaOTs*XOTBytes);
	}

	for(i=0;i<numLaOTs;i++)
	{
		if(x0[i].type!=ShareAndMac)
		{
			if(c[i].ShareAndMac.value) 
				nnobPRG(padder, c[i].ShareAndMac.mac, NNOB_KEY_BYTES, *nonce, mc[i], X1[i], XOTBytes);
			else nnobPRG(padder, c[i].ShareAndMac.mac, NNOB_KEY_BYTES, *nonce, mc[i], X0[i], XOTBytes);
			*nonce+=1;
			z[i].ShareAndMac.value = mc[i][0];
			if(c[i].ShareAndMac.value) memcpy(temp, x1[i].key, NNOB_KEY_BYTES);	
			else memcpy(temp, x0[i].key, NNOB_KEY_BYTES);	
			if(z[i].ShareAndMac.value) memxor(temp, globalDelta, NNOB_KEY_BYTES);	
			success &= memcmp(mc[i]+1, temp, NNOB_KEY_BYTES)==0?true:false;
			assert(success);
			d[i] = z[i].ShareAndMac.value^r[i].ShareAndMac.value;
		}
	}

	if(x0[0].type==ShareAndMac)
	{
		orecv(pd, destparty, d, numLaOTs);
	}
	else
	{
		osend(pd, destparty, d, numLaOTs);
	}
	for(i=0;i<numLaOTs;i++)
	{
		z[i]=r[i];
		nnobSetBitXorConst(pd, &z[i], d[i]);

		if(x0[0].type==ShareAndMac)
		{
			nnobHash(padder, z[i].key, NNOB_KEY_BYTES, *nonce, I0[i], T1[i]);
			memcpy(temp, z[i].key, NNOB_KEY_BYTES);
			memxor(temp, globalDelta, NNOB_KEY_BYTES);
			nnobHash(padder, temp, NNOB_KEY_BYTES, *nonce, I1[i], T0[i]);
			*nonce+=1;
		}
	}

	if(x0[0].type==ShareAndMac)
	{
		osend(pd, destparty, I0, numLaOTs*NNOB_KEY_BYTES);
		osend(pd, destparty, I1, numLaOTs*NNOB_KEY_BYTES);
	}
	else
	{
		orecv(pd, destparty, I0, numLaOTs*NNOB_KEY_BYTES);
		orecv(pd, destparty, I1, numLaOTs*NNOB_KEY_BYTES);
	}

	for(i=0;i<numLaOTs;i++)
	{
		if(x0[0].type!=ShareAndMac)
		{
			if(z[i].ShareAndMac.value)
			{
				memcpy(T1[i], mc[i]+1+NNOB_KEY_BYTES, NNOB_KEY_BYTES);	
				nnobHash(padder, z[i].ShareAndMac.mac, NNOB_KEY_BYTES, *nonce, 
						T0[i], I1[i]);	
			}
			else
			{
				memcpy(T0[i], mc[i]+1+NNOB_KEY_BYTES, NNOB_KEY_BYTES);	
				nnobHash(padder, z[i].ShareAndMac.mac, NNOB_KEY_BYTES, *nonce, 
						T1[i], I0[i]);	
			}
			*nonce+=1;
		}
	}
	success &= ocEqualityCheck(pd, gen, T0, numLaOTs*NNOB_KEY_BYTES, destparty);
	success &= ocEqualityCheck(pd, gen, T1, numLaOTs*NNOB_KEY_BYTES, destparty);
	free(X0);
	free(X1);
	free(I0);
	free(I1);
	free(T0);
	free(T1);
	free(mc);
	free(d);
	return success;
}

void LaOTCombine(ProtocolDesc* pd,  
		NnobHalfBit* x10, const NnobHalfBit* x20, 
		NnobHalfBit* x11,const NnobHalfBit* x21, 
		NnobHalfBit* c1, const NnobHalfBit* c2, 
		NnobHalfBit* z1, const NnobHalfBit* z2,
		int destparty)
{
	assert((x10->type==x11->type) && 
			(x11->type==x20->type) &&
			(x20->type==x21->type));
	assert((c1->type==c2->type) &&
			(c2->type==z1->type) &&
			(z1->type==z2->type));
	assert(x10->type!=c1->type);

	bool d;

	if(x10->type)
	{
		d=x10->ShareAndMac.value^x11->ShareAndMac.value^
			x20->ShareAndMac.value^x21->ShareAndMac.value;
		osend(pd, destparty, &d, 1);
	}
	else
	{
		orecv(pd, destparty, &d, 1);
	}
	
	nnobSetBitXor(pd, z1, z2);
	if(d) nnobSetBitXor(pd, z1, c1); 
	nnobSetBitXor(pd, c1, c2);

	*x11=*x10;
	nnobSetBitXor(pd, x11, x21);
	nnobSetBitXor(pd, x10, x20);
}

bool aOT(ProtocolDesc *pd, NnobProtocolDesc* npd, NnobHalfBitType outputType, int bucketSize, 
		int destparty)
{
	NnobHalfBit* x0; 
	NnobHalfBit* x1; 
	NnobHalfBit* c;
	NnobHalfBit* z;
	int n;
	if(outputType == ShareAndMac)
	{
		x0 = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x0;
		x1 = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x1;
		c = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.c;
		z = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.z;
		n = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.n;
	}
	else {
		x0 = npd->FDeal.aOTQuadruple.aOTKeyOfZ.x0;
		x1 = npd->FDeal.aOTQuadruple.aOTKeyOfZ.x1;
		c = npd->FDeal.aOTQuadruple.aOTKeyOfZ.c;
		z = npd->FDeal.aOTQuadruple.aOTKeyOfZ.z;
		n = npd->FDeal.aOTQuadruple.aOTKeyOfZ.n;
	}

	int i,k;
	int numLaOTs = n*bucketSize;
	bool success = true;
	unsigned* permutation = malloc(numLaOTs*sizeof(unsigned));
	NnobHalfBit* LaOTx0 = malloc(numLaOTs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaOTx1 = malloc(numLaOTs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaOTc = malloc(numLaOTs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaOTr = malloc(numLaOTs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaOTz = malloc(numLaOTs*sizeof(NnobHalfBit)); 
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGen();
	int nonce = 0;

	for(i=0;i<numLaOTs;i++)
	{
		randomOblivAuthentication(npd, &LaOTx0[i], !outputType);
		randomOblivAuthentication(npd, &LaOTx1[i], !outputType);
		randomOblivAuthentication(npd, &LaOTc[i], outputType);
		randomOblivAuthentication(npd, &LaOTr[i], outputType);
	}
	success &= LaOT(pd, npd, LaOTx0, LaOTx1, LaOTc, LaOTr, LaOTz,
			padder, gen, &nonce, numLaOTs, destparty);	
	if(outputType==ShareAndMac)
	{
		bcRandomPermutation(gen, permutation, numLaOTs);	
		osend(pd, destparty, permutation, numLaOTs*sizeof(unsigned));
	}
	else orecv(pd, destparty, permutation, numLaOTs*sizeof(unsigned));
	for(i=0;i<n;i++)
	{
		x0[i]=LaOTx0[permutation[i]];
		x1[i]=LaOTx1[permutation[i]];
		c[i]=LaOTc[permutation[i]];
		z[i]=LaOTz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++)
	{
		for(i=0;i<n;i++)
		{
			LaOTCombine(pd, &x0[i], &LaOTx0[permutation[i+k*n]],
					&x1[i], &LaOTx1[permutation[i+k*n]],
					&c[i], &LaOTc[permutation[i+k*n]],
					&z[i], &LaOTz[permutation[i+k*n]],
					destparty);
		}
	}
	free(permutation);
	releaseBCipherRandomGen(gen);
	releaseBCipherRandomGen(padder);
	free(LaOTx0);
	free(LaOTx1);
	free(LaOTc);
	free(LaOTr);
	free(LaOTz);
	return success;
}

bool LaAND(ProtocolDesc* pd, NnobProtocolDesc* npd, const NnobHalfBit* x, 
		const NnobHalfBit* y, const NnobHalfBit* r, NnobHalfBit* z, 
		BCipherRandomGen* padder, BCipherRandomGen* gen, int* nonce, int numLaANDs, int destparty)
{
	assert(x[0].type==y[0].type && y[0].type==r[0].type);
	int i;
	bool* d = malloc(numLaANDs);
	char (*U)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char (*V)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char temp[2*NNOB_KEY_BYTES];
	char temp2[NNOB_KEY_BYTES];
	char zeroes[NNOB_KEY_BYTES];
	memset(zeroes, 0, NNOB_KEY_BYTES);
	const char* globalDelta = npd->globalDelta;
	bool success = true;

	for(i=0;i<numLaANDs;i++)
	{
		if(x[0].type==ShareAndMac)
		{
			z[i].ShareAndMac.value = x[i].ShareAndMac.value && y[i].ShareAndMac.value;	
			d[i] = z[i].ShareAndMac.value^r[i].ShareAndMac.value;
		}
	}

	if(x[0].type==ShareAndMac)
	{
		osend(pd, destparty, d, numLaANDs);
	}
	else
	{
		orecv(pd, destparty, d, numLaANDs);
	}
	for(i=0;i<numLaANDs;i++)
	{
		z[i]=r[i];
		nnobSetBitXorConst(pd, &z[i], d[i]);
		/*assert(z[i].ShareAndMac.value == (x[i].ShareAndMac.value && y[i].ShareAndMac.value));*/
		if(x[0].type!=ShareAndMac)
		{
			memcpy(temp, x[i].key, NNOB_KEY_BYTES);
			memcpy(temp+NNOB_KEY_BYTES, z[i].key, NNOB_KEY_BYTES);
			nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, temp2, zeroes);

			memcpy(temp, x[i].key, NNOB_KEY_BYTES);
			memxor(temp, globalDelta, NNOB_KEY_BYTES);
			memcpy(temp+NNOB_KEY_BYTES, y[i].key, NNOB_KEY_BYTES);
			memxor(temp+NNOB_KEY_BYTES, z[i].key, NNOB_KEY_BYTES);
			nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, U[i], temp2);
		}
	}
	if(x[0].type==ShareAndMac)
	{
		
		orecv(pd, destparty, U, NNOB_KEY_BYTES*numLaANDs);
	}
	else
	{
		osend(pd, destparty, U, NNOB_KEY_BYTES*numLaANDs);
	}
	for(i=0;i<numLaANDs;i++)
	{
		if(x[0].type==ShareAndMac)
		{
			memcpy(temp, x[i].ShareAndMac.mac, NNOB_KEY_BYTES);
			memcpy(temp+NNOB_KEY_BYTES, z[i].ShareAndMac.mac, NNOB_KEY_BYTES);
			if(x[i].ShareAndMac.value)
			{
				memxor(temp+NNOB_KEY_BYTES, y[i].ShareAndMac.mac, NNOB_KEY_BYTES);
				nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, V[i], U[i]);
			}
			else
			{
				nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, V[i], zeroes);
			}
			*nonce+=1;
		}
		else
		{
			memcpy(V[i], temp2, NNOB_KEY_BYTES);
			*nonce+=1;
		}

	}
	success &= ocEqualityCheck(pd, gen, V, numLaANDs*NNOB_KEY_BYTES, destparty);
	free(d);
	free(U);
	free(V);
	return success;
}

void LaANDCombine(ProtocolDesc* pd, 
		NnobHalfBit* x1, const NnobHalfBit* x2,  
		NnobHalfBit* y1, const NnobHalfBit* y2, 
		NnobHalfBit* z1, const NnobHalfBit* z2, 
		int destparty)
{
	assert(x1->type==x2->type);
	assert(y1->type==y2->type);
	assert(z1->type==z2->type);
	assert(x1->type==y1->type);
	assert(y1->type==z1->type);

	bool d;	

	if(x1->type==ShareAndMac)
	{
		d=y1->ShareAndMac.value^y2->ShareAndMac.value;	
		osend(pd, destparty, &d, 1);
	}
	else
	{
		orecv(pd, destparty, &d, 1);
	}
	nnobSetBitXor(pd, z1, z2);
	if(d) nnobSetBitXor(pd, z1, x2);
	nnobSetBitXor(pd, x1, x2);

}


bool aAND(ProtocolDesc *pd, NnobProtocolDesc* npd, NnobHalfBitType type, int bucketSize,
		int destparty)
{
	NnobHalfBit* x; 
	NnobHalfBit* y; 
	NnobHalfBit* z;
	int n;
	if(type == ShareAndMac)
	{
		x = npd->FDeal.aANDTriple.aANDShareAndMac.x;
		y = npd->FDeal.aANDTriple.aANDShareAndMac.y;
		z = npd->FDeal.aANDTriple.aANDShareAndMac.z;
		n = npd->FDeal.aANDTriple.aANDShareAndMac.n;
	}
	else {
		x = npd->FDeal.aANDTriple.aANDKey.x;
		y = npd->FDeal.aANDTriple.aANDKey.y;
		z = npd->FDeal.aANDTriple.aANDKey.z;
		n = npd->FDeal.aANDTriple.aANDKey.n;
	}

	int i,k;
	int numLaANDs = n*bucketSize;
	bool success = true;
	unsigned* permutation = malloc(numLaANDs*sizeof(unsigned));
	NnobHalfBit* LaANDx = malloc(numLaANDs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaANDy = malloc(numLaANDs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaANDr = malloc(numLaANDs*sizeof(NnobHalfBit)); 
	NnobHalfBit* LaANDz = malloc(numLaANDs*sizeof(NnobHalfBit)); 
	char dummy[NNOB_HASH_ALGO_KEYBYTES];
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGenByAlgoKey(NNOB_HASH_ALGO, dummy);
	int nonce = 0;


	for(i=0;i<numLaANDs;i++)
	{
		randomOblivAuthentication(npd, &LaANDx[i], type);
		randomOblivAuthentication(npd, &LaANDy[i], type);
		randomOblivAuthentication(npd, &LaANDr[i], type);
	}
	success &= LaAND(pd, npd, LaANDx, LaANDy, LaANDr, LaANDz,
			padder, gen, &nonce, numLaANDs, destparty);	
	assert(success);

	if(type==ShareAndMac)
	{
		bcRandomPermutation(gen, permutation, numLaANDs);	
		osend(pd, destparty, permutation, numLaANDs*sizeof(unsigned));
	}
	else orecv(pd, destparty, permutation, numLaANDs*sizeof(unsigned));

	for(i=0;i<n;i++)
	{
		x[i]=LaANDx[permutation[i]];
		y[i]=LaANDy[permutation[i]];
		z[i]=LaANDz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++)
	{
		for(i=0;i<n;i++)
		{
			LaANDCombine(pd, &x[i], &LaANDx[permutation[i+k*n]],
					&y[i], &LaANDy[permutation[i+k*n]],
					&z[i], &LaANDz[permutation[i+k*n]],
					destparty);
		}
	}
	free(permutation);
	releaseBCipherRandomGen(gen);
	releaseBCipherRandomGen(padder);
	free(LaANDx);
	free(LaANDy);
	free(LaANDr);
	free(LaANDz);
	return success;
}

void getRandomAOTQuadruple(NnobProtocolDesc* npd, 
		NnobHalfBit* x0, NnobHalfBit* x1, NnobHalfBit* c, NnobHalfBit* z, NnobHalfBitType zType)
{
	int counter;
	if(zType==Key)
	{
		counter = npd->FDeal.aOTQuadruple.aOTKeyOfZ.counter;
		assert(counter<npd->FDeal.aOTQuadruple.aOTKeyOfZ.n);
		npd->FDeal.aOTQuadruple.aOTKeyOfZ.counter++;

		*x0=npd->FDeal.aOTQuadruple.aOTKeyOfZ.x0[counter];
		*x1=npd->FDeal.aOTQuadruple.aOTKeyOfZ.x1[counter];
		*c=npd->FDeal.aOTQuadruple.aOTKeyOfZ.c[counter];
		*z=npd->FDeal.aOTQuadruple.aOTKeyOfZ.z[counter];
	}
	else
	{
		counter = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.counter;
		assert(counter<npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.n);
		npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.counter++;

		*x0=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x0[counter];
		*x1=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x1[counter];
		*c=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.c[counter];
		*z=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.z[counter];
	}
}

void getRandomAANDTriple(NnobProtocolDesc* npd, 
		NnobHalfBit* x, NnobHalfBit* y, NnobHalfBit* z, NnobHalfBitType type)
{
	int counter;
	if(type==Key)
	{
		counter = npd->FDeal.aANDTriple.aANDKey.counter;
		assert(counter<npd->FDeal.aANDTriple.aANDKey.n);
		npd->FDeal.aANDTriple.aANDKey.counter++;

		*x=npd->FDeal.aANDTriple.aANDKey.x[counter];
		*y=npd->FDeal.aANDTriple.aANDKey.y[counter];
		*z=npd->FDeal.aANDTriple.aANDKey.z[counter];
	}
	else
	{
		counter = npd->FDeal.aANDTriple.aANDShareAndMac.counter;
		assert(counter<npd->FDeal.aANDTriple.aANDShareAndMac.n);
		npd->FDeal.aANDTriple.aANDShareAndMac.counter++;

		*x=npd->FDeal.aANDTriple.aANDShareAndMac.x[counter];
		*y=npd->FDeal.aANDTriple.aANDShareAndMac.y[counter];
		*z=npd->FDeal.aANDTriple.aANDShareAndMac.z[counter];
	}
}
