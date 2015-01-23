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
	if(x->nnob.shareAndMac.share)
	{
		memcpy(temp, x->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
		memxor(temp, globalDeltaToYKey, NNOB_KEY_BYTES);
		success&=memcmp(temp, y->nnob.key.key, NNOB_KEY_BYTES)==0?true:false;
	}
	else success&=memcmp(x->nnob.shareAndMac.mac, y->nnob.key.key, NNOB_KEY_BYTES)==0?true:false;
	if(y->nnob.shareAndMac.share)
	{
		memcpy(temp, y->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
		memxor(temp, globalDeltaToXKey, NNOB_KEY_BYTES);
		success&=memcmp(temp, x->nnob.key.key, NNOB_KEY_BYTES)==0?true:false;
		assert(success);
	}
	else success&=memcmp(y->nnob.shareAndMac.mac, x->nnob.key.key, NNOB_KEY_BYTES)==0?true:false;
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
	bool b = bit->nnob.shareAndMac.share%2==1?true:false;
	fprintf(stderr, "{share=%d, mac=", b);
	debugPrintHex(bit->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
	print(", key=");
	debugPrintHex(bit->nnob.key.key, NNOB_KEY_BYTES);
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
/*static int logfloor(int x,int b)*/
/*{ int res=0;*/
  /*while(b<=x) { x/=b; res++; }*/
  /*return res;*/
/*}*/

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

void
bitmatMul(char* dest,const char* mat,const char* src,int rows,int cols);

typedef struct
{
  char (*dest)[NNOB_KEY_BYTES];
  char *hashmat,(*src)[A_BIT_PARAMETER_BYTES] ;
  int from,to;
} BitMatMulThread;


void* bitmatMul_thread_nnob(void* args)
{
  BitMatMulThread* a=args;
  int i;
  for(i=a->from;i<a->to;i++)
    bitmatMul(a->dest[i],a->hashmat,a->src[i],
              8*NNOB_KEY_BYTES,8*A_BIT_PARAMETER_BYTES);
  return NULL;
}


typedef struct TransposeThread {
	int* rows;
	char*  src;
	char (*dest)[A_BIT_PARAMETER_BYTES];
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
			setBit(a->dest[i], j, getBit(boxColumn,i));
		}
	}
	return NULL;
}
bool WaBitBoxGetBitAndMac(ProtocolDesc* pd, bool* b,
		char* mat, char (*aBitFullMac)[A_BIT_PARAMETER_BYTES],
		int n, OTExtValidation validation, int destparty){
	assert(n%8==0);
	int i;
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

	free(mask);
	free(rows);
	free(box);
	recverExtensionBoxRelease(r);
	releaseBCipherRandomGen(gen);
	return success;
}

bool WaBitBoxGetKey(ProtocolDesc* pd, nnob_key_t globalDelta,
		char* mat, char (*aBitFullKey)[A_BIT_PARAMETER_BYTES],
		int n, OTExtValidation validation, int destparty){
	assert(n%8==0);
	int i;
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

	free(rows);
	free(box);
	senderExtensionBoxRelease(s);
	releaseBCipherRandomGen(gen);
	return success;
}

void WaBitToaBit(char (*aBit)[NNOB_KEY_BYTES], char (*WaBit)[A_BIT_PARAMETER_BYTES], char* mat, int n){
	int rowBytes = (n+7)/8, i,done=0,tc;
	BitMatMulThread args[OT_THREAD_COUNT];
	pthread_t hasht[OT_THREAD_COUNT];
	if(8*rowBytes<=OT_THREAD_THRESHOLD) tc=1;
	else tc=OT_THREAD_COUNT;

	for(i=0;i<tc;++i)
	{
		int c = (n-done)/(tc-i); 
		args[i] = (BitMatMulThread){.dest=aBit,.hashmat=mat,.src=WaBit,
			.from=done,.to=done+c};
		if(i==tc-1) bitmatMul_thread_nnob(&args[i]); // no thread if tc==1
		else pthread_create(&hasht[i],NULL,bitmatMul_thread_nnob,&args[i]);
		done+=c;
	}
	for(i=0;i<tc-1;++i) pthread_join(hasht[i],NULL);
}


void randomOblivAuthenticationShareAndMac(NnobProtocolDesc* npd, NnobShareAndMac* sm) {
	int counter = npd->aBitsShareAndMac.counter;
	npd->aBitsShareAndMac.counter+=1;
	assert(counter<npd->aBitsShareAndMac.n);
	sm->share = npd->aBitsShareAndMac.share[counter];
	memcpy(sm->mac, npd->aBitsShareAndMac.mac[counter], NNOB_KEY_BYTES);
}

void randomOblivAuthenticationKey(NnobProtocolDesc* npd, NnobKey* k) {
	int counter = npd->aBitsKey.counter;
	npd->aBitsKey.counter+=1;
	assert(counter<npd->aBitsKey.n);
	memcpy(k->key, npd->aBitsKey.key[counter], NNOB_KEY_BYTES);
}

void static inline nnobShareAndMacXOR(NnobShareAndMac* dest, const NnobShareAndMac* src) {
	dest->share^= src->share;
	memxor(dest->mac, src->mac, NNOB_KEY_BYTES);
}
void static inline nnobKeyXOR(NnobKey* dest, const NnobKey* src) {
	memxor(dest->key, src->key, NNOB_KEY_BYTES);
}

void static inline nnobShareAndMacXORConst(NnobShareAndMac* output, bool src) {
	if(!src) return;
	output->share^= src;
}

void static inline nnobKeyXORConst(NnobKey* output, bool src, const char* globalDelta) {
	if(!src) return;
	memxor(output->key, globalDelta, NNOB_KEY_BYTES);
}
 
void nnobSetBitXor(ProtocolDesc* pd, NnobHalfBit* dest, const NnobHalfBit* src) {
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

/*bool nnobSetBitAndGetXY(ProtocolDesc* pd, NnobProtocolDesc* npd, */
		/*NnobHalfBit* xy, const NnobHalfBit* x, const NnobHalfBit* y, int destparty)*/
/*{*/
	/*assert(x->type==y->type);*/
	/*[>print("x in xy: ");<]*/
	/*[>debugPrintNnobHalfBit(x);<]*/
	/*[>nl();<]*/

	/*[>print("y in xy: ");<]*/
	/*[>debugPrintNnobHalfBit(y);<]*/
	/*[>nl();<]*/
	/*bool success = true;*/
	
	/*NnobHalfBit u;*/
	/*NnobHalfBit v;*/
	/*NnobHalfBit w;*/
	/*NnobHalfBit f;*/
	/*NnobHalfBit g;*/

	/*NnobHalfBitType type = x->type;*/

	/*getRandomAANDTriple(npd, &u, &v, &w, type);*/
	/*if(type==ShareAndMac)*/
	/*{*/
		/*f=u;*/
		/*nnobSetBitXor(pd, &f, x);*/
		/*g=v;*/
		/*nnobSetBitXor(pd, &g, y);*/
		/*osend(pd, destparty, &f, sizeof(NnobHalfBit));	*/
		/*osend(pd, destparty, &g, sizeof(NnobHalfBit));	*/
	/*}*/
	/*else*/
	/*{*/
		/*nnob_key_t fMac;*/
		/*nnob_key_t gMac;*/

		/*orecv(pd, destparty, &f, sizeof(NnobHalfBit));	*/
		/*orecv(pd, destparty, &g, sizeof(NnobHalfBit));	*/
		/*assert(u.type==Key);*/
		/*memcpy(fMac, u.key, NNOB_KEY_BYTES);*/
		/*memxor(fMac, x->key, NNOB_KEY_BYTES);*/
		/*if(f.ShareAndMac.value) memxor(fMac, npd->globalDelta, NNOB_KEY_BYTES); */
		/*success&=memcmp(fMac, f.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;*/


		/*assert(v.type==Key);*/
		/*memcpy(gMac, v.key, NNOB_KEY_BYTES);*/
		/*memxor(gMac, y->key, NNOB_KEY_BYTES);*/
		/*if(g.ShareAndMac.value) memxor(gMac, npd->globalDelta, NNOB_KEY_BYTES); */
		/*success&=memcmp(gMac, g.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;*/
	/*}*/

	/**xy = w;*/
	/*if(f.ShareAndMac.value) nnobSetBitXor(pd, xy, y);*/
	/*if(g.ShareAndMac.value) nnobSetBitXor(pd, xy, x);*/
	/*if(f.ShareAndMac.value&&g.ShareAndMac.value) */
		/*nnobSetBitXorConst(pd, xy, true);*/


	/*[>print("xy in xy: ");<]*/
	/*[>if(xy->type==Key)<]*/
	/*[>{<]*/
		/*[>debugPrintNnobHalfBitExtra(xy, npd->globalDelta);<]*/
	/*[>}<]*/
	/*[>else<]*/
	/*[>{<]*/
		/*[>debugPrintNnobHalfBit(xy);<]*/
	/*[>}	<]*/
	/*[>nl();<]*/
	/*return success;*/
/*}*/

/*bool nnobSetBitAndGetS(ProtocolDesc* pd, NnobProtocolDesc* npd,*/
		/*NnobHalfBit* sYours, const NnobHalfBit* r, const NnobHalfBit* x, */
		/*const NnobHalfBit* y, int destparty)*/
/*{*/
	/*assert(x->type!=y->type);*/

	/*[>print("x in s: ");<]*/
	/*[>debugPrintNnobHalfBit(x);<]*/
	/*[>nl();<]*/

	/*[>print("y in s: ");<]*/
	/*[>debugPrintNnobHalfBit(y);<]*/
	/*[>nl();<]*/
	/*[>print("r in s: ");<]*/
	/*[>debugPrintNnobHalfBit(r);<]*/
	/*[>nl();<]*/
	/*NnobHalfBit u0;	*/
	/*NnobHalfBit u1;	*/
	/*NnobHalfBit c;	*/
	/*NnobHalfBit w;	*/
	/*NnobHalfBit d;*/
	/*NnobHalfBit f;*/
	/*NnobHalfBit g;*/
	/*bool success = true;*/

	/*NnobHalfBitType type = x->type;*/

	/*getRandomAOTQuadruple(npd, &u0, &u1, &c, &w, !type);*/
	/*[>print("u0 in s: ");<]*/
	/*[>debugPrintNnobHalfBit(&u0);<]*/
	/*[>nl();<]*/
	/*[>print("u1 in s: ");<]*/
	/*[>debugPrintNnobHalfBit(&u1);<]*/
	/*[>nl();<]*/
	/*[>print("c in s: ");<]*/
	/*[>debugPrintNnobHalfBit(&c);<]*/
	/*[>nl();<]*/
	/*[>print("w in s: ");<]*/
	/*[>debugPrintNnobHalfBit(&w);<]*/
	/*[>nl();<]*/

	/*if(type==ShareAndMac)*/
	/*{*/
		/*nnob_key_t dMac;*/
		/*orecv(pd, destparty, &d, sizeof(NnobHalfBit));*/
		/*assert(c.type==Key);*/
		/*memcpy(dMac, c.key, NNOB_KEY_BYTES);*/
		/*memxor(dMac, y->key, NNOB_KEY_BYTES);*/
		/*if(d.ShareAndMac.value) memxor(dMac, npd->globalDelta, NNOB_KEY_BYTES);*/
		/*success&=memcmp(dMac, d.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;*/
		/*assert(success);*/

		/*f=u0;*/
		/*nnobSetBitXor(pd, &f, &u1);*/
		/*nnobSetBitXor(pd, &f, x);*/
		/*g=u0;*/
		/*nnobSetBitXor(pd, &g, r);*/
		/*if(d.ShareAndMac.value) nnobSetBitXor(pd, &g, x);*/
		/*osend(pd, destparty, &f, sizeof(NnobHalfBit));*/
		/*osend(pd, destparty, &g, sizeof(NnobHalfBit));*/
	/*}*/
	/*else*/
	/*{*/
		/*nnob_key_t fMac;*/
		/*nnob_key_t gMac;*/
		/*d=c;*/
		/*nnobSetBitXor(pd, &d, y);*/
		/*[>fprintf(stderr, "d = c^y = %d\n", d);<]*/
		/*osend(pd, destparty, &d, sizeof(NnobHalfBit));*/

		/*orecv(pd, destparty, &f, sizeof(NnobHalfBit));*/
		/*orecv(pd, destparty, &g, sizeof(NnobHalfBit));*/
		/*assert(u0.type==Key);*/
		/*assert(u1.type==Key);*/
		/*memcpy(fMac, u0.key, NNOB_KEY_BYTES);*/
		/*memxor(fMac, u1.key, NNOB_KEY_BYTES);*/
		/*memxor(fMac, x->key, NNOB_KEY_BYTES);*/
		/*if(f.ShareAndMac.value) memxor(fMac, npd->globalDelta, NNOB_KEY_BYTES);*/
		/*success&=memcmp(fMac, f.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;*/

		/*assert(r->type==Key);*/
		/*memcpy(gMac, r->key, NNOB_KEY_BYTES);*/
		/*memxor(gMac, u0.key, NNOB_KEY_BYTES);*/
		/*if(d.ShareAndMac.value) memxor(gMac, x->key, NNOB_KEY_BYTES);*/
		/*if(g.ShareAndMac.value) memxor(gMac, npd->globalDelta, NNOB_KEY_BYTES);*/
		/*success&=memcmp(gMac, g.ShareAndMac.mac, NNOB_KEY_BYTES)==0?true:false;*/
		/*assert(success);*/
	/*}*/
	/**sYours=w;*/
	/*if(f.ShareAndMac.value) nnobSetBitXor(pd, sYours, &c);*/
	/*nnobSetBitXorConst(pd, sYours, g.ShareAndMac.value);*/

	/*[>print("sYours in s: ");<]*/
	/*[>if(sYours->type==Key)<]*/
	/*[>{<]*/
		/*[>debugPrintNnobHalfBitExtra(sYours, npd->globalDelta);<]*/
	/*[>}<]*/
	/*[>else<]*/
	/*[>{<]*/
		/*[>debugPrintNnobHalfBit(sYours);<]*/
	/*[>}	<]*/
	/*[>nl();<]*/
	/*return success;*/
/*}*/

/*bool nnobSetBitAnd(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit *y)*/
/*{*/

	/*NnobProtocolDesc* npd = pd->extra;*/
	/*assert(npd!=NULL);*/

	/*bool success=true;*/

	/*NnobHalfBit xMac;*/
	/*xMac.type = ShareAndMac;*/
	/*xMac.ShareAndMac.value =  x->nnob.shareAndMac.share;*/
	/*memcpy(xMac.ShareAndMac.mac, x->nnob.shareAndMac.mac, NNOB_KEY_BYTES);*/

	/*NnobHalfBit xKey;*/
	/*xKey.type = Key;*/
	/*memcpy(xKey.key, x->nnob.key.key, NNOB_KEY_BYTES);*/
	/*print("xKey: ");*/
	/*debugPrintNnobHalfBit(&xKey);*/
	/*nl();*/

	/*NnobHalfBit yMac;*/
	/*yMac.type = ShareAndMac;*/
	/*yMac.ShareAndMac.value =  y->nnob.shareAndMac.share;*/
	/*memcpy(yMac.ShareAndMac.mac, y->nnob.shareAndMac.mac, NNOB_KEY_BYTES);*/
	/*print("yMac: ");*/
	/*debugPrintNnobHalfBit(&yMac);*/
	/*nl();*/

	/*NnobHalfBit yKey;*/
	/*yKey.type = Key;*/
	/*memcpy(yKey.key, y->nnob.key.key, NNOB_KEY_BYTES);*/

	/*NnobHalfBit zMac;	*/
	/*NnobHalfBit zKey;	*/
	/*NnobHalfBit rMac;	*/
	/*NnobHalfBit rKey;	*/
	/*NnobHalfBit xyMac;	*/
	/*NnobHalfBit xyKey;	*/
	/*NnobHalfBit sMac;	*/
	/*NnobHalfBit sKey;	*/

	/*randomOblivAuthentication(npd, &rMac, ShareAndMac);*/
	/*randomOblivAuthentication(npd, &rKey, Key);*/

	/*int destparty = (pd->thisParty*2)%3;*/

	/*if(pd->thisParty==1)*/
	/*{*/
		/*success&=nnobSetBitAndGetXY(pd, npd, &xyMac, &xMac, &yMac, destparty);*/
		/*success&=nnobSetBitAndGetS(pd, npd, &sKey, &rMac, &xMac, &yKey, destparty);*/

		/*success&=nnobSetBitAndGetXY(pd, npd, &xyKey, &xKey, &yKey, destparty);*/
		/*success&=nnobSetBitAndGetS(pd, npd, &sMac, &rKey, &xKey, &yMac, destparty);*/
	/*}*/
	/*else*/
	/*{*/
		/*success&=nnobSetBitAndGetXY(pd, npd, &xyKey, &xKey, &yKey, destparty);*/
		/*success&=nnobSetBitAndGetS(pd, npd, &sMac, &rKey, &xKey, &yMac, destparty);*/

		/*success&=nnobSetBitAndGetXY(pd, npd, &xyMac, &xMac, &yMac, destparty);*/
		/*success&=nnobSetBitAndGetS(pd, npd, &sKey, &rMac, &xMac, &yKey, destparty);*/

	/*}*/

	/*zMac=rMac;*/
	/*nnobSetBitXor(pd, &zMac, &sMac);*/
	/*nnobSetBitXor(pd, &zMac, &xyMac);*/

	/*zKey=rKey;*/
	/*nnobSetBitXor(pd, &zKey, &sKey);*/
	/*nnobSetBitXor(pd, &zKey, &xyKey);*/

	/*z->nnob.shareAndMac.share = zMac.ShareAndMac.value;*/
	/*memcpy(z->nnob.shareAndMac.mac,zMac.ShareAndMac.mac, NNOB_KEY_BYTES);*/
	/*memcpy(z->nnob.key.key,zKey.key, NNOB_KEY_BYTES);*/
	/*return success;*/
/*}*/

void nnobHalfBittoOblivBit(OblivBit* obit, NnobHalfBit* hbitMac, NnobHalfBit* hbitKey)
{
	assert(hbitMac->type==ShareAndMac);
	assert(hbitKey->type==Key);
	obit->nnob.shareAndMac.share = hbitMac->ShareAndMac.value;
	memcpy(obit->nnob.shareAndMac.mac, hbitMac->ShareAndMac.mac, NNOB_KEY_BYTES);
	memcpy(obit->nnob.key.key, hbitKey->key, NNOB_KEY_BYTES);
}

/*void nnobGetOblivInputSendShare(ProtocolDesc* pd, NnobProtocolDesc* npd, bool input,*/
		/*OblivBit* myInput)*/
/*{*/
	/*NnobHalfBit myInputKey;*/
	/*NnobHalfBit myInputMac;*/
	/*bool b;*/
	/*int destparty = (pd->thisParty*3)%2;*/
	/*randomOblivAuthentication(npd, &myInputMac, ShareAndMac);	*/
	/*b = input ^ myInputMac.ShareAndMac.value;*/
	/*osend(pd, destparty, &b, 1);*/
	/*myInputKey.type = Key;*/
	/*memset(myInputKey.key, 0, NNOB_KEY_BYTES);*/
	/*nnobSetBitXorConst(pd, &myInputKey, b);*/
	/*nnobHalfBittoOblivBit(myInput, &myInputMac, &myInputKey);*/
/*}*/

/*void nnobGetOblivInputRecvShare(ProtocolDesc* pd, NnobProtocolDesc* npd, OblivBit* yourInput)*/
/*{*/
	/*NnobHalfBit yourInputKey;*/
	/*NnobHalfBit yourInputMac;*/
	/*bool b;*/
	/*int destparty = (pd->thisParty*3)%2;*/
	/*randomOblivAuthentication(npd, &yourInputKey, Key);	*/
	/*orecv(pd, destparty, &b, 1);*/
	/*yourInputMac.type = ShareAndMac;*/
	/*yourInputMac.ShareAndMac.value = b;*/
	/*memset(yourInputMac.ShareAndMac.mac, 0, NNOB_KEY_BYTES);*/
	/*nnobHalfBittoOblivBit(yourInput, &yourInputMac, &yourInputKey);*/
/*}*/

/*void nnobGetOblivInput(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* input,*/
		/*OblivBit* x, OblivBit* y, int numOblivInput)*/
/*{*/
	/*int i;*/
	/*int destparty = (pd->thisParty*3)%2;*/

	/*NnobHalfBit* yourInputKey = malloc(numOblivInput*sizeof(NnobHalfBit));*/
	/*NnobHalfBit* yourInputMac = malloc(numOblivInput*sizeof(NnobHalfBit));*/
	
	/*NnobHalfBit* myInputKey = malloc(numOblivInput*sizeof(NnobHalfBit));*/
	/*NnobHalfBit* myInputMac = malloc(numOblivInput*sizeof(NnobHalfBit));*/

	/*bool* b = malloc(numOblivInput);*/

	/*for(i=0;i<numOblivInput;i++)*/
	/*{*/
		/*if(pd->thisParty==1)*/
		/*{*/
			/*randomOblivAuthentication(npd, &myInputMac[i], ShareAndMac);	*/
			/*b[i] = input[i] ^ myInputMac[i].ShareAndMac.value;*/
			/*myInputKey[i].type = Key;*/
			/*memset(myInputKey[i].key, 0, NNOB_KEY_BYTES);*/
			/*nnobSetBitXorConst(pd, &myInputKey[i], b[i]);*/
			/*nnobHalfBittoOblivBit(&x[i], &myInputMac[i], &myInputKey[i]);*/
		/*}*/
		/*else*/
		/*{*/
			/*randomOblivAuthentication(npd, &yourInputKey[i], Key);	*/
		/*}*/
	/*}*/
	/*if(pd->thisParty==1)*/
	/*{*/
		/*osend(pd, destparty, b, numOblivInput);*/
	/*}*/
	/*else*/
	/*{*/
		/*orecv(pd, destparty, b, numOblivInput);*/
	/*}*/
	/*for(i=0;i<numOblivInput;i++)*/
	/*{*/
		/*if(pd->thisParty!=1)*/
		/*{*/
			/*yourInputMac[i].type = ShareAndMac;*/
			/*yourInputMac[i].ShareAndMac.value = b[i];*/
			/*memset(yourInputMac[i].ShareAndMac.mac, 0, NNOB_KEY_BYTES);*/
			/*nnobHalfBittoOblivBit(&x[i], &yourInputMac[i], &yourInputKey[i]);*/
		/*}*/
	/*}*/
	
	
	/*for(i=0;i<numOblivInput;i++)*/
	/*{*/
		/*if(pd->thisParty==1)*/
		/*{*/
			/*randomOblivAuthentication(npd, &yourInputKey[i], Key);	*/
		/*}*/
		/*else*/
		/*{*/
			/*randomOblivAuthentication(npd, &myInputMac[i], ShareAndMac);	*/
			/*b[i] = input[i] ^ myInputMac[i].ShareAndMac.value;*/
			/*myInputKey[i].type = Key;*/
			/*memset(myInputKey[i].key, 0, NNOB_KEY_BYTES);*/
			/*nnobSetBitXorConst(pd, &myInputKey[i], b[i]);*/
			/*nnobHalfBittoOblivBit(&y[i], &myInputMac[i], &myInputKey[i]);*/
		/*}*/
	/*}*/
	/*if(pd->thisParty==1)*/
	/*{*/
		/*orecv(pd, destparty, b, numOblivInput);*/
	/*}*/
	/*else*/
	/*{*/
		/*osend(pd, destparty, b, numOblivInput);*/
	/*}*/
	/*for(i=0;i<numOblivInput;i++)*/
	/*{*/
		/*if(pd->thisParty==1)*/
		/*{*/
			/*yourInputMac[i].type = ShareAndMac;*/
			/*yourInputMac[i].ShareAndMac.value = b[i];*/
			/*memset(yourInputMac[i].ShareAndMac.mac, 0, NNOB_KEY_BYTES);*/
			/*nnobHalfBittoOblivBit(&y[i], &yourInputMac[i], &yourInputKey[i]);*/
		/*}*/
	/*}*/

	/*free(yourInputKey);*/
	/*free(yourInputMac);*/
	/*free(myInputKey);*/
	/*free(myInputMac);*/
	/*free(b);*/

	/*[>if(pd->thisParty==1)<]*/
	/*[>{<]*/
		/*[>nnobGetOblivInputSendShare(pd, npd, input, x);<]*/
		/*[>nnobGetOblivInputRecvShare(pd, npd, y);<]*/
	/*[>}<]*/
	/*[>else<]*/
	/*[>{<]*/
		/*[>nnobGetOblivInputRecvShare(pd, npd, x);<]*/
		/*[>nnobGetOblivInputSendShare(pd, npd, input, y);<]*/
	/*[>}<]*/
/*}*/

void nnobSendOblivInput(ProtocolDesc* pd, bool* input, OblivBit* oblivInput, int numOblivInput)
{
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2; 
	bool* b = malloc(numOblivInput);
	char const* globalDelta = npd->globalDelta;
	for(i=0;i<numOblivInput;i++) {
		randomOblivAuthenticationShareAndMac(npd, &oblivInput[i].nnob.shareAndMac);	
		b[i] = input[i] ^ oblivInput[i].nnob.shareAndMac.share;
		memset(oblivInput[i].nnob.key.key, 0, NNOB_KEY_BYTES);
		nnobKeyXORConst(&oblivInput[i].nnob.key, b[i], globalDelta);
	}
	osend(pd, destparty, b, numOblivInput);
	free(b);
}

void nnobRecvOblivInput(ProtocolDesc* pd, OblivBit* oblivInput, int numOblivInput)
{
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2; 
	bool* b = malloc(numOblivInput);
	orecv(pd, destparty, b, numOblivInput);
	for(i=0;i<numOblivInput;i++) {
		randomOblivAuthenticationKey(npd, &oblivInput[i].nnob.key);	
		oblivInput[i].nnob.shareAndMac.share = b[i];
		memset(oblivInput[i].nnob.shareAndMac.mac, 0, NNOB_KEY_BYTES);
	}
	free(b);
}

bool nnobRevealOblivBit(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* output, 
		OblivBit* input)
{
	bool success = true;
	nnob_key_t inputMac;
	bool inputBit;
	if(pd->thisParty==1)
	{
		osend(pd, 2, input->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
		osend(pd, 2, &input->nnob.shareAndMac.share, 1);

		orecv(pd, 2, inputMac, NNOB_KEY_BYTES);
		orecv(pd, 2, &inputBit, 1);
		if(inputBit) memxor(inputMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(inputMac, input->nnob.key.key,NNOB_KEY_BYTES)==0?true:false;
	}
	else
	{
		orecv(pd, 1, inputMac, NNOB_KEY_BYTES);
		orecv(pd, 1, &inputBit, 1);
		if(inputBit) memxor(inputMac, npd->globalDelta, NNOB_KEY_BYTES);
		success&=memcmp(inputMac, input->nnob.key.key,NNOB_KEY_BYTES)==0?true:false;

		osend(pd, 1, input->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
		osend(pd, 1, &input->nnob.shareAndMac.share, 1);
	}
	*output = inputBit^input->nnob.shareAndMac.share;
	assert(success);
	return success;
}

bool LaOTShareAndMacOfZ(ProtocolDesc* pd, const NnobKey* x0, 
		const NnobKey* x1, const NnobShareAndMac* c,
		const NnobShareAndMac* r, NnobShareAndMac* z,  BCipherRandomGen* padder, 
		BCipherRandomGen* gen, int* nonce, int numLaOTs) {

	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2;
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

	orecv(pd, destparty, X0, numLaOTs*XOTBytes);
	orecv(pd, destparty, X1, numLaOTs*XOTBytes);
	for(i=0;i<numLaOTs;i++) {
		if(c[i].share) 
			nnobPRG(padder, c[i].mac, NNOB_KEY_BYTES, *nonce, mc[i], X1[i], XOTBytes);
		else nnobPRG(padder, c[i].mac, NNOB_KEY_BYTES, *nonce, mc[i], X0[i], XOTBytes);
		*nonce+=1;
		z[i].share = mc[i][0];
		if(c[i].share) memcpy(temp, x1[i].key, NNOB_KEY_BYTES);	
		else memcpy(temp, x0[i].key, NNOB_KEY_BYTES);	
		if(z[i].share) memxor(temp, globalDelta, NNOB_KEY_BYTES);	
		success &= memcmp(mc[i]+1, temp, NNOB_KEY_BYTES)==0?true:false;
		assert(success);
		d[i] = z[i].share^r[i].share;
	}
	osend(pd, destparty, d, numLaOTs);
	for(i=0;i<numLaOTs;i++) {
		z[i]=r[i];
		nnobShareAndMacXORConst(&z[i], d[i]);
	}
	orecv(pd, destparty, I0, numLaOTs*NNOB_KEY_BYTES);
	orecv(pd, destparty, I1, numLaOTs*NNOB_KEY_BYTES);

	for(i=0;i<numLaOTs;i++) {
		if(z[i].share) {
			memcpy(T1[i], mc[i]+1+NNOB_KEY_BYTES, NNOB_KEY_BYTES);	
			nnobHash(padder, z[i].mac, NNOB_KEY_BYTES, *nonce, 
					T0[i], I1[i]);	
		}
		else {
			memcpy(T0[i], mc[i]+1+NNOB_KEY_BYTES, NNOB_KEY_BYTES);	
			nnobHash(padder, z[i].mac, NNOB_KEY_BYTES, *nonce, 
					T1[i], I0[i]);	
		}
		*nonce+=1;
	}
	success &= ocEqualityCheck(pd, gen, T0, numLaOTs*NNOB_KEY_BYTES, destparty);
	success &= ocEqualityCheck(pd, gen, T1, numLaOTs*NNOB_KEY_BYTES, destparty);
	free(X0); free(X1); free(I0); free(I1); free(T0); free(T1); free(mc); free(d);
	return success;
}

bool LaOTKeyOfZ(ProtocolDesc* pd, const NnobShareAndMac* x0, 
		const NnobShareAndMac* x1, const NnobKey* c,
		const NnobKey* r, NnobKey* z,  BCipherRandomGen* padder, 
		BCipherRandomGen* gen, int* nonce, int numLaOTs) {

	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2;
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
	
	for(i=0;i<numLaOTs;i++) {
		char m0[XOTBytes], m1[XOTBytes];
		randomizeBuffer(gen, T0[i], NNOB_KEY_BYTES);
		randomizeBuffer(gen, T1[i], NNOB_KEY_BYTES);

		m0[0] = x0[i].share;
		memcpy(m0+1, x0[i].mac, NNOB_KEY_BYTES);
		if(x0[i].share) memcpy(m0+1+NNOB_KEY_BYTES, T1[i], NNOB_KEY_BYTES);
		else memcpy(m0+1+NNOB_KEY_BYTES, T0[i], NNOB_KEY_BYTES);

		m1[0] = x1[i].share;
		memcpy(m1+1, x1[i].mac, NNOB_KEY_BYTES);
		if(x1[i].share) memcpy(m1+1+NNOB_KEY_BYTES, T1[i], NNOB_KEY_BYTES);
		else memcpy(m1+1+NNOB_KEY_BYTES, T0[i], NNOB_KEY_BYTES);

		nnobPRG(padder, c[i].key, NNOB_KEY_BYTES, *nonce, X0[i], m0, XOTBytes);	
		memcpy(temp, c[i].key, NNOB_KEY_BYTES);
		memxor(temp, globalDelta, NNOB_KEY_BYTES);
		nnobPRG(padder, temp, NNOB_KEY_BYTES, *nonce, X1[i], m1, XOTBytes);	
		*nonce+=1;
	}
	osend(pd, destparty, X0, numLaOTs*XOTBytes);
	osend(pd, destparty, X1, numLaOTs*XOTBytes);
	orecv(pd, destparty, d, numLaOTs);
	for(i=0;i<numLaOTs;i++) {
		z[i]=r[i];
		nnobKeyXORConst(&z[i], d[i], globalDelta);
		nnobHash(padder, z[i].key, NNOB_KEY_BYTES, *nonce, I0[i], T1[i]);
		memcpy(temp, z[i].key, NNOB_KEY_BYTES);
		memxor(temp, globalDelta, NNOB_KEY_BYTES);
		nnobHash(padder, temp, NNOB_KEY_BYTES, *nonce, I1[i], T0[i]);
		*nonce+=1;
	}
	osend(pd, destparty, I0, numLaOTs*NNOB_KEY_BYTES);
	osend(pd, destparty, I1, numLaOTs*NNOB_KEY_BYTES);
	success &= ocEqualityCheck(pd, gen, T0, numLaOTs*NNOB_KEY_BYTES, destparty);
	success &= ocEqualityCheck(pd, gen, T1, numLaOTs*NNOB_KEY_BYTES, destparty);
	free(X0); free(X1); free(I0); free(I1); free(T0); free(T1); free(mc); free(d);
	return success;
}

void LaOTCombineShareAndMacOfZ(ProtocolDesc* pd,  
		NnobKey* x10, const NnobKey* x20, 
		NnobKey* x11, const NnobKey* x21, 
		NnobShareAndMac* c1, const NnobShareAndMac* c2, 
		NnobShareAndMac* z1, const NnobShareAndMac* z2) {
	int destparty = pd->thisParty==1?1:2;
	bool d;
	orecv(pd, destparty, &d, 1);
	nnobShareAndMacXOR(z1, z2);
	if(d) nnobShareAndMacXOR(z1, c1); 
	nnobShareAndMacXOR(c1, c2);
	*x11=*x10; // may be a problem
	nnobKeyXOR(x11, x21);
	nnobKeyXOR(x10, x20);
}

void LaOTCombineKeyOfZ(ProtocolDesc* pd,  
		NnobShareAndMac* x10, const NnobShareAndMac* x20, 
		NnobShareAndMac* x11, const NnobShareAndMac* x21, 
		NnobKey* c1, const NnobKey* c2, 
		NnobKey* z1, const NnobKey* z2) {
	bool d;
	int destparty = pd->thisParty==1?1:2;
	d=x10->share^x11->share^
		x20->share^x21->share;
	osend(pd, destparty, &d, 1);
	
	nnobKeyXOR(z1, z2);
	if(d) nnobKeyXOR(z1, c1); 
	nnobKeyXOR(c1, c2);

	*x11=*x10; // may be a problem
	nnobShareAndMacXOR(x11, x21);
	nnobShareAndMacXOR(x10, x20);
}

bool aOTShareAndMacOfZ(ProtocolDesc *pd, AOTShareAndMacOfZ* sm) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i,k, bucketSize = npd->bucketSize, n = sm->n, numLaOTs = sm->n*bucketSize, 
		destparty = pd->thisParty==1?1:2;
	bool success = true;
	unsigned* permutation = malloc(numLaOTs*sizeof(unsigned));
	NnobKey* LaOTx0 = malloc(numLaOTs*sizeof(NnobKey)); 
	NnobKey* LaOTx1 = malloc(numLaOTs*sizeof(NnobKey)); 
	NnobShareAndMac* LaOTc = malloc(numLaOTs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaOTr = malloc(numLaOTs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaOTz = malloc(numLaOTs*sizeof(NnobShareAndMac)); 
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGen();
	int nonce = 0;

	for(i=0;i<numLaOTs;i++)
	{
		randomOblivAuthenticationKey(npd, &LaOTx0[i]);
		randomOblivAuthenticationKey(npd, &LaOTx1[i]);
		randomOblivAuthenticationShareAndMac(npd, &LaOTc[i]);
		randomOblivAuthenticationShareAndMac(npd, &LaOTr[i]);
	}
	success &= LaOTShareAndMacOfZ(pd, LaOTx0, LaOTx1, LaOTc, LaOTr, LaOTz,
			padder, gen, &nonce, numLaOTs);	
	bcRandomPermutation(gen, permutation, numLaOTs);	
	osend(pd, destparty, permutation, numLaOTs*sizeof(unsigned));
	for(i=0;i<n;i++) {
		sm->x0[i]=LaOTx0[permutation[i]];
		sm->x1[i]=LaOTx1[permutation[i]];
		sm->c[i]=LaOTc[permutation[i]];
		sm->z[i]=LaOTz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++) {
		for(i=0;i<n;i++) {
			LaOTCombineShareAndMacOfZ(pd, &sm->x0[i], &LaOTx0[permutation[i+k*n]],
					&sm->x1[i], &LaOTx1[permutation[i+k*n]],
					&sm->c[i], &LaOTc[permutation[i+k*n]],
					&sm->z[i], &LaOTz[permutation[i+k*n]]);
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

bool aOTKeyOfZ(ProtocolDesc *pd, AOTKeyOfZ* key) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i,k,n = key->n, bucketSize = npd->bucketSize,
		numLaOTs = key->n*bucketSize, destparty = pd->thisParty==1?1:2;
	bool success = true;
	unsigned* permutation = malloc(numLaOTs*sizeof(unsigned));
	NnobShareAndMac* LaOTx0 = malloc(numLaOTs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaOTx1 = malloc(numLaOTs*sizeof(NnobShareAndMac)); 
	NnobKey* LaOTc = malloc(numLaOTs*sizeof(NnobKey)); 
	NnobKey* LaOTr = malloc(numLaOTs*sizeof(NnobKey)); 
	NnobKey* LaOTz = malloc(numLaOTs*sizeof(NnobKey)); 
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGen();
	int nonce = 0;

	for(i=0;i<numLaOTs;i++) {
		randomOblivAuthenticationShareAndMac(npd, &LaOTx0[i]);
		randomOblivAuthenticationShareAndMac(npd, &LaOTx1[i]);
		randomOblivAuthenticationKey(npd, &LaOTc[i]);
		randomOblivAuthenticationKey(npd, &LaOTr[i]);
	}
	success &= LaOTKeyOfZ(pd, LaOTx0, LaOTx1, LaOTc, LaOTr, LaOTz,
			padder, gen, &nonce, numLaOTs);	
	orecv(pd, destparty, permutation, numLaOTs*sizeof(unsigned));
	for(i=0;i<n;i++) {
		key->x0[i]=LaOTx0[permutation[i]];
		key->x1[i]=LaOTx1[permutation[i]];
		key->c[i]=LaOTc[permutation[i]];
		key->z[i]=LaOTz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++) {
		for(i=0;i<n;i++) {
			LaOTCombineKeyOfZ(pd, &key->x0[i], &LaOTx0[permutation[i+k*n]],
					&key->x1[i], &LaOTx1[permutation[i+k*n]],
					&key->c[i], &LaOTc[permutation[i+k*n]],
					&key->z[i], &LaOTz[permutation[i+k*n]]);
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

bool LaANDShareAndMac(ProtocolDesc* pd, 
		const NnobShareAndMac* x, const NnobShareAndMac* y, 
		const NnobShareAndMac* r, NnobShareAndMac* z, 
		BCipherRandomGen* padder, BCipherRandomGen* gen, 
		int* nonce, int numLaANDs) {

	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2;
	bool* d = malloc(numLaANDs);
	char (*U)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char (*V)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char temp[2*NNOB_KEY_BYTES];
	char zeroes[NNOB_KEY_BYTES];
	memset(zeroes, 0, NNOB_KEY_BYTES);
	bool success = true;

	for(i=0;i<numLaANDs;i++) {
		z[i].share = x[i].share && y[i].share;	
		d[i] = z[i].share^r[i].share;
	}
	osend(pd, destparty, d, numLaANDs);
	for(i=0;i<numLaANDs;i++) {
		z[i]=r[i];
		nnobShareAndMacXORConst(&z[i], d[i]);
		/*assert(z[i].ShareAndMac.value == (x[i].ShareAndMac.value && y[i].ShareAndMac.value));*/
	}
	orecv(pd, destparty, U, NNOB_KEY_BYTES*numLaANDs);
	for(i=0;i<numLaANDs;i++) {
		memcpy(temp, x[i].mac, NNOB_KEY_BYTES);
		memcpy(temp+NNOB_KEY_BYTES, z[i].mac, NNOB_KEY_BYTES);
		if(x[i].share) {
			memxor(temp+NNOB_KEY_BYTES, y[i].mac, NNOB_KEY_BYTES);
			nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, V[i], U[i]);
		}
		else {
			nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, V[i], zeroes);
		}
		*nonce+=1;
	}
	success &= ocEqualityCheck(pd, gen, V, numLaANDs*NNOB_KEY_BYTES, destparty);
	free(d); free(U); free(V);
	return success;
}

bool LaANDKey(ProtocolDesc* pd, 
		const NnobKey* x, const NnobKey* y, 
		const NnobKey* r, NnobKey* z, 
		BCipherRandomGen* padder, BCipherRandomGen* gen, 
		int* nonce, int numLaANDs) {

	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i, destparty = pd->thisParty==1?1:2;
	bool* d = malloc(numLaANDs);
	char (*U)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char (*V)[NNOB_KEY_BYTES] = malloc(numLaANDs*NNOB_KEY_BYTES);
	char temp[2*NNOB_KEY_BYTES];
	char temp2[NNOB_KEY_BYTES];
	char zeroes[NNOB_KEY_BYTES];
	memset(zeroes, 0, NNOB_KEY_BYTES);
	const char* globalDelta = npd->globalDelta;
	bool success = true;
	orecv(pd, destparty, d, numLaANDs);
	for(i=0;i<numLaANDs;i++)
	{
		z[i]=r[i];
		nnobKeyXORConst(&z[i], d[i], globalDelta);
		/*assert(z[i].ShareAndMac.value == (x[i].ShareAndMac.value && y[i].ShareAndMac.value));*/
		memcpy(temp, x[i].key, NNOB_KEY_BYTES);
		memcpy(temp+NNOB_KEY_BYTES, z[i].key, NNOB_KEY_BYTES);
		nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, temp2, zeroes);

		memcpy(temp, x[i].key, NNOB_KEY_BYTES);
		memxor(temp, globalDelta, NNOB_KEY_BYTES);
		memcpy(temp+NNOB_KEY_BYTES, y[i].key, NNOB_KEY_BYTES);
		memxor(temp+NNOB_KEY_BYTES, z[i].key, NNOB_KEY_BYTES);
		nnobHash(padder, temp, 2*NNOB_KEY_BYTES, *nonce, U[i], temp2);
	}
	osend(pd, destparty, U, NNOB_KEY_BYTES*numLaANDs);
	for(i=0;i<numLaANDs;i++) {
		memcpy(V[i], temp2, NNOB_KEY_BYTES);
		*nonce+=1;
	}
	success &= ocEqualityCheck(pd, gen, V, numLaANDs*NNOB_KEY_BYTES, destparty);
	free(d); free(U); free(V);
	return success;
}

void LaANDCombineShareAndMac(ProtocolDesc* pd, 
		NnobShareAndMac* x1, const NnobShareAndMac* x2,  
		NnobShareAndMac* y1, const NnobShareAndMac* y2, 
		NnobShareAndMac* z1, const NnobShareAndMac* z2) {

	bool d;	
	int destparty = pd->thisParty==1?1:2;
	d=y1->share^y2->share;	
	osend(pd, destparty, &d, 1);
	nnobShareAndMacXOR(z1, z2);
	if(d) nnobShareAndMacXOR(z1, x2);
	nnobShareAndMacXOR(x1, x2);
}

void LaANDCombineKey(ProtocolDesc* pd, 
		NnobKey* x1, const NnobKey* x2,  
		NnobKey* y1, const NnobKey* y2, 
		NnobKey* z1, const NnobKey* z2) {

	bool d;	
	int destparty = pd->thisParty==1?1:2;

	orecv(pd, destparty, &d, 1);
	nnobKeyXOR(z1, z2);
	if(d) nnobKeyXOR(z1, x2);
	nnobKeyXOR(x1, x2);
}

bool aANDShareAndMac(ProtocolDesc *pd, AANDShareAndMac* sm) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i,k,n = sm->n,bucketSize = npd->bucketSize, numLaANDs = n*bucketSize,
		nonce = 0, destparty = pd->thisParty==1?1:2;
	bool success = true;
	unsigned* permutation = malloc(numLaANDs*sizeof(unsigned));
	NnobShareAndMac* LaANDx = malloc(numLaANDs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaANDy = malloc(numLaANDs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaANDr = malloc(numLaANDs*sizeof(NnobShareAndMac)); 
	NnobShareAndMac* LaANDz = malloc(numLaANDs*sizeof(NnobShareAndMac)); 
	char dummy[NNOB_HASH_ALGO_KEYBYTES];
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGenByAlgoKey(NNOB_HASH_ALGO, dummy);

	for(i=0;i<numLaANDs;i++) {
		randomOblivAuthenticationShareAndMac(npd, &LaANDx[i]);
		randomOblivAuthenticationShareAndMac(npd, &LaANDy[i]);
		randomOblivAuthenticationShareAndMac(npd, &LaANDr[i]);
	}
	success &= LaANDShareAndMac(pd, LaANDx, LaANDy, LaANDr, LaANDz,
			padder, gen, &nonce, numLaANDs);	
	assert(success);

	bcRandomPermutation(gen, permutation, numLaANDs);	
	osend(pd, destparty, permutation, numLaANDs*sizeof(unsigned));

	for(i=0;i<n;i++) {
		sm->x[i]=LaANDx[permutation[i]];
		sm->y[i]=LaANDy[permutation[i]];
		sm->z[i]=LaANDz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++) {
		for(i=0;i<n;i++) {
			LaANDCombineShareAndMac(pd, &sm->x[i], &LaANDx[permutation[i+k*n]],
					&sm->y[i], &LaANDy[permutation[i+k*n]],
					&sm->z[i], &LaANDz[permutation[i+k*n]]);
		}
	}
	free(permutation);
	releaseBCipherRandomGen(gen);
	releaseBCipherRandomGen(padder);
	free(LaANDx); free(LaANDy); free(LaANDr); free(LaANDz);
	return success;
}

bool aANDKey(ProtocolDesc *pd, AANDKey* key) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int i,k,n = key->n,bucketSize = npd->bucketSize, numLaANDs = n*bucketSize,
		nonce = 0, destparty = pd->thisParty==1?1:2;
	bool success = true;
	unsigned* permutation = malloc(numLaANDs*sizeof(unsigned));
	NnobKey* LaANDx = malloc(numLaANDs*sizeof(NnobKey)); 
	NnobKey* LaANDy = malloc(numLaANDs*sizeof(NnobKey)); 
	NnobKey* LaANDr = malloc(numLaANDs*sizeof(NnobKey)); 
	NnobKey* LaANDz = malloc(numLaANDs*sizeof(NnobKey)); 
	char dummy[NNOB_HASH_ALGO_KEYBYTES];
	BCipherRandomGen* gen = newBCipherRandomGen();
	BCipherRandomGen* padder = newBCipherRandomGenByAlgoKey(NNOB_HASH_ALGO, dummy);

	for(i=0;i<numLaANDs;i++) {
		randomOblivAuthenticationKey(npd, &LaANDx[i]);
		randomOblivAuthenticationKey(npd, &LaANDy[i]);
		randomOblivAuthenticationKey(npd, &LaANDr[i]);
	}
	success &= LaANDKey(pd, LaANDx, LaANDy, LaANDr, LaANDz,
			padder, gen, &nonce, numLaANDs);	
	assert(success);

	orecv(pd, destparty, permutation, numLaANDs*sizeof(unsigned));

	for(i=0;i<n;i++) {
		key->x[i]=LaANDx[permutation[i]];
		key->y[i]=LaANDy[permutation[i]];
		key->z[i]=LaANDz[permutation[i]];
	}

	for(k=1;k<bucketSize;k++) {
		for(i=0;i<n;i++) {
			LaANDCombineKey(pd, &key->x[i], &LaANDx[permutation[i+k*n]],
					&key->y[i], &LaANDy[permutation[i+k*n]],
					&key->z[i], &LaANDz[permutation[i+k*n]]);
		}
	}
	free(permutation);
	releaseBCipherRandomGen(gen);
	releaseBCipherRandomGen(padder);
	free(LaANDx); free(LaANDy); free(LaANDr); free(LaANDz);
	return success;
}

NnobProtocolDesc* initNnobProtocolDesc(ProtocolDesc* pd, int numOTs, 
		OTExtValidation validation) {
	/*int denom = logfloor(numOTs, 2)+1;*/
	/*int bucketSize = (int)(1 + (NNOB_KEY_BYTES*8)/denom);*/
	int bucketSize = BUCKET_SIZE;
	int allOTs = (OTS_PER_aOT+OTS_PER_aAND)*bucketSize+OTS_FOR_R+OTS_FOR_INPUTS;
	NnobProtocolDesc* npd = malloc(sizeof(NnobProtocolDesc));
	pd->extra=npd;
	npd->bucketSize = bucketSize;
	int n = ((numOTs+7)/8)*8, aOTs = n/allOTs, aANDs = n/allOTs;
	int	destparty = pd->thisParty==1?1:2;
	assert(aOTs!=0);
	assert(aANDs!=0);

	npd->aBitsShareAndMac.counter = 0;
	npd->aBitsShareAndMac.n = n;
	npd->aBitsShareAndMac.share = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsShareAndMac.mac = malloc(n*NNOB_KEY_BYTES);

	npd->aBitsKey.key = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsKey.counter = 0;
	npd->aBitsKey.n = n;

	npd->FDeal.aOTShareAndMacOfZ.x0 = malloc(aOTs*sizeof(NnobKey));
	npd->FDeal.aOTShareAndMacOfZ.x1 = malloc(aOTs*sizeof(NnobKey));
	npd->FDeal.aOTShareAndMacOfZ.c = malloc(aOTs*sizeof(NnobShareAndMac));
	npd->FDeal.aOTShareAndMacOfZ.z = malloc(aOTs*sizeof(NnobShareAndMac));
	npd->FDeal.aOTShareAndMacOfZ.counter = 0;
	npd->FDeal.aOTShareAndMacOfZ.n = aOTs;

	npd->FDeal.aOTKeyOfZ.x0 = malloc(aOTs*sizeof(NnobShareAndMac));
	npd->FDeal.aOTKeyOfZ.x1 = malloc(aOTs*sizeof(NnobShareAndMac));
	npd->FDeal.aOTKeyOfZ.c = malloc(aOTs*sizeof(NnobKey));
	npd->FDeal.aOTKeyOfZ.z = malloc(aOTs*sizeof(NnobKey));
	npd->FDeal.aOTKeyOfZ.counter = 0;
	npd->FDeal.aOTKeyOfZ.n = aOTs;

	npd->FDeal.aANDShareAndMac.x = malloc(aANDs*sizeof(NnobShareAndMac));
	npd->FDeal.aANDShareAndMac.y = malloc(aANDs*sizeof(NnobShareAndMac));
	npd->FDeal.aANDShareAndMac.z = malloc(aANDs*sizeof(NnobShareAndMac));
	npd->FDeal.aANDShareAndMac.counter = 0;
	npd->FDeal.aANDShareAndMac.n = aANDs;

	npd->FDeal.aANDKey.x = malloc(aANDs*sizeof(NnobKey));
	npd->FDeal.aANDKey.y = malloc(aANDs*sizeof(NnobKey));
	npd->FDeal.aANDKey.z = malloc(aANDs*sizeof(NnobKey));
	npd->FDeal.aANDKey.counter = 0;
	npd->FDeal.aANDKey.n = aANDs;

	npd->error = false;

	char mat1[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char mat2[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char (*aBitFullMac)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	char (*aBitFullKey)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	if(destparty==1)
	{
		npd->error |= WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat1, aBitFullMac, numOTs, validation, destparty);
		npd->error |= WaBitBoxGetKey(pd, npd->globalDelta, 
				mat2, aBitFullKey, numOTs, validation, destparty);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat1, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat2, numOTs);
		npd->error |= aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
		npd->error |= aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
		npd->error |= aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
		npd->error |= aANDKey(pd, &npd->FDeal.aANDKey);
	}
	else
	{
		npd->error |= WaBitBoxGetKey(pd, npd->globalDelta, 
				mat1, aBitFullKey, numOTs, validation, destparty);
		npd->error |= WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat2, aBitFullMac, numOTs, validation, destparty);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat2, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat1, numOTs);
		npd->error |= aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
		npd->error |= aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
		npd->error |= aANDKey(pd, &npd->FDeal.aANDKey);
		npd->error |= aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
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
	free(npd->FDeal.aOTShareAndMacOfZ.x0); 
	free(npd->FDeal.aOTShareAndMacOfZ.x1); 
	free(npd->FDeal.aOTShareAndMacOfZ.c); 
	free(npd->FDeal.aOTShareAndMacOfZ.z); 
	free(npd->FDeal.aOTKeyOfZ.x0);
	free(npd->FDeal.aOTKeyOfZ.x1);
	free(npd->FDeal.aOTKeyOfZ.c);
	free(npd->FDeal.aOTKeyOfZ.z);
	free(npd->FDeal.aANDShareAndMac.x);
	free(npd->FDeal.aANDShareAndMac.y);
	free(npd->FDeal.aANDShareAndMac.z);
	free(npd->FDeal.aANDKey.x);
	free(npd->FDeal.aANDKey.y);
	free(npd->FDeal.aANDKey.z);
	free(npd);
}

/*void getRandomAOTQuadruple(NnobProtocolDesc* npd, */
		/*NnobHalfBit* x0, NnobHalfBit* x1, NnobHalfBit* c, NnobHalfBit* z, NnobHalfBitType zType)*/
/*{*/
	/*int counter;*/
	/*if(zType==Key)*/
	/*{*/
		/*counter = npd->FDeal.aOTQuadruple.aOTKeyOfZ.counter;*/
		/*assert(counter<npd->FDeal.aOTQuadruple.aOTKeyOfZ.n);*/
		/*npd->FDeal.aOTQuadruple.aOTKeyOfZ.counter++;*/

		/**x0=npd->FDeal.aOTQuadruple.aOTKeyOfZ.x0[counter];*/
		/**x1=npd->FDeal.aOTQuadruple.aOTKeyOfZ.x1[counter];*/
		/**c=npd->FDeal.aOTQuadruple.aOTKeyOfZ.c[counter];*/
		/**z=npd->FDeal.aOTQuadruple.aOTKeyOfZ.z[counter];*/
	/*}*/
	/*else*/
	/*{*/
		/*counter = npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.counter;*/
		/*assert(counter<npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.n);*/
		/*npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.counter++;*/

		/**x0=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x0[counter];*/
		/**x1=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.x1[counter];*/
		/**c=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.c[counter];*/
		/**z=npd->FDeal.aOTQuadruple.aOTShareAndMacOfZ.z[counter];*/
	/*}*/
/*}*/
/*void getRandomAANDTriple(NnobProtocolDesc* npd, */
		/*NnobHalfBit* x, NnobHalfBit* y, NnobHalfBit* z, NnobHalfBitType type)*/
/*{*/
	/*int counter;*/
	/*if(type==Key)*/
	/*{*/
		/*counter = npd->FDeal.aANDTriple.aANDKey.counter;*/
		/*assert(counter<npd->FDeal.aANDTriple.aANDKey.n);*/
		/*npd->FDeal.aANDTriple.aANDKey.counter++;*/

		/**x=npd->FDeal.aANDTriple.aANDKey.x[counter];*/
		/**y=npd->FDeal.aANDTriple.aANDKey.y[counter];*/
		/**z=npd->FDeal.aANDTriple.aANDKey.z[counter];*/
	/*}*/
	/*else*/
	/*{*/
		/*counter = npd->FDeal.aANDTriple.aANDShareAndMac.counter;*/
		/*assert(counter<npd->FDeal.aANDTriple.aANDShareAndMac.n);*/
		/*npd->FDeal.aANDTriple.aANDShareAndMac.counter++;*/

		/**x=npd->FDeal.aANDTriple.aANDShareAndMac.x[counter];*/
		/**y=npd->FDeal.aANDTriple.aANDShareAndMac.y[counter];*/
		/**z=npd->FDeal.aANDTriple.aANDShareAndMac.z[counter];*/
	/*}*/
/*}*/
