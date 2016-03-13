#ifdef ENABLE_NNOB
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

void static inline nnobKeyXORConst(NnobKey* output, bool src, const char* globalDelta);
bool debugMatchinKeyShareMac(const NnobShareAndMac* sm, const NnobKey* k, const char* globalDelta) {
	NnobKey temp = *k;	
	nnobKeyXORConst(&temp, sm->share, globalDelta);
	return memcmp(temp.key, sm->mac, NNOB_KEY_BYTES)==0;
}

void debugPrintOblivBit(const OblivBit* bit)
{
	bool b = bit->nnob.shareAndMac.share%2==1?true:false;
	fprintf(stderr, "{share=%d, mac=", b);
	debugPrintHex(bit->nnob.shareAndMac.mac, NNOB_KEY_BYTES);
	print(", key=");
	debugPrintHex(bit->nnob.key.key, NNOB_KEY_BYTES);
	fprintf(stderr,", unknown=%d", bit->unknown);
	fprintf(stderr,", knownValue=%d", bit->knownValue);
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
  char keyx[gen->klen];
  assert(klen<=gen->klen);
  memcpy(keyx,key,klen); memset(keyx+klen,0,gen->klen-klen);
  resetBCipherRandomGen(gen,keyx);
  setctrFromIntBCipherRandomGen(gen,nonce);
  randomizeBuffer(gen,dest,n);
  memxor(dest, src, n);
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

void randomOblivAuthentication(NnobProtocolDesc* npd, OblivBit* bit) {
	randomOblivAuthenticationShareAndMac(npd, &bit->nnob.shareAndMac);	
	randomOblivAuthenticationKey(npd, &bit->nnob.key);	
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

void nnobXOR(OblivBit* z, const OblivBit* x, const OblivBit* y) {
	OblivBit newZ = *x;
	newZ=*x;
	nnobShareAndMacXOR(&newZ.nnob.shareAndMac, &y->nnob.shareAndMac);
	nnobKeyXOR(&newZ.nnob.key, &y->nnob.key);
	*z=newZ;
}

void debugNnobSendOblivInput(ProtocolDesc* pd, bool* input, OblivBit* oblivInput, 
		int numOblivInput)
{
	NnobProtocolDesc* npd = pd->extra;
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

void debugNnobRecvOblivInput(ProtocolDesc* pd, OblivBit* oblivInput, int numOblivInput)
{
	NnobProtocolDesc* npd = pd->extra;
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
		NnobShareAndMac* z1, const NnobShareAndMac* z2, 
		const unsigned* permutation, int n, int k) {
	int i,pos, destparty = pd->thisParty==1?1:2;
	bool* d = malloc(n);
	orecv(pd, destparty, d, n);
	for(i=0;i<n;i++) {
		pos = permutation[i+k*n];	
		nnobShareAndMacXOR(&z1[i], &z2[pos]);
		if(d[i]) nnobShareAndMacXOR(&z1[i],&c1[i]); 
		nnobShareAndMacXOR(&c1[i], &c2[pos]);
		x11[i]=x10[i]; 
		nnobKeyXOR(&x11[i], &x21[pos]);
		nnobKeyXOR(&x10[i], &x20[pos]);
	}
	free(d);
}

void LaOTCombineKeyOfZ(ProtocolDesc* pd,  
		NnobShareAndMac* x10, const NnobShareAndMac* x20, 
		NnobShareAndMac* x11, const NnobShareAndMac* x21, 
		NnobKey* c1, const NnobKey* c2, 
		NnobKey* z1, const NnobKey* z2, 
		const unsigned* permutation, int n, int k) {
	bool* d = malloc(n);
	int i,pos,destparty = pd->thisParty==1?1:2;
	for(i=0;i<n;i++) {
		pos = permutation[i+k*n];
		d[i]=x10[i].share^x11[i].share^
			x20[pos].share^x21[pos].share;
	}
	osend(pd, destparty, d, n);
	for(i=0;i<n;i++) {
		pos = permutation[i+k*n];
		nnobKeyXOR(&z1[i], &z2[pos]);
		if(d[i]) nnobKeyXOR(&z1[i], &c1[i]); 
		nnobKeyXOR(&c1[i], &c2[pos]);
		x11[i]=x10[i];
		nnobShareAndMacXOR(&x11[i], &x21[pos]);
		nnobShareAndMacXOR(&x10[i], &x20[pos]);
	}
	free(d);
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
		LaOTCombineShareAndMacOfZ(pd, sm->x0, LaOTx0,
				sm->x1, LaOTx1,
				sm->c, LaOTc,
				sm->z, LaOTz,
				permutation, n, k);
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
		LaOTCombineKeyOfZ(pd, key->x0, LaOTx0,
				key->x1, LaOTx1,
				key->c, LaOTc,
				key->z, LaOTz,
				permutation, n, k);
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

void setupFDeal(NnobProtocolDesc* npd, int numOTs) {
	int n = ((numOTs+7)/8)*8;
	int allOTs = (OTS_PER_aOT+OTS_PER_aAND)*npd->bucketSize+OTS_FOR_R+OTS_FOR_INPUTS;
	int aOTs = n/allOTs, aANDs = n/allOTs;
	assert(aOTs!=0);
	assert(aANDs!=0);
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
}


void hashMacCheck(BCipherRandomGen* gen, int* nonce, nnob_key_t* tar, const nnob_key_t* src) {
	char keyx[gen->klen];
	int keylen = 2*NNOB_KEY_BYTES;
	memcpy(keyx,src,NNOB_KEY_BYTES); 
	memcpy(keyx+NNOB_KEY_BYTES,tar,NNOB_KEY_BYTES); 
	memset(keyx+keylen,0,gen->klen-keylen);
	resetBCipherRandomGen(gen,keyx);
	setctrFromIntBCipherRandomGen(gen,*nonce);
	randomizeBuffer(gen,*tar,NNOB_KEY_BYTES);
	(*nonce)+=1;
	debugPrintHex(*tar, NNOB_KEY_BYTES);
	nl();
}

void hashMacCheckShareAndMac(NnobProtocolDesc* npd, const NnobShareAndMac* sm) {
	hashMacCheck(npd->gen, &npd->nonce, &npd->cumulativeHashCheckMac, &sm->mac);
}

void hashMacCheckKey(NnobProtocolDesc* npd, const NnobKey* key, bool b) {
	if(b) {
		nnob_key_t* checkKey = malloc(sizeof(nnob_key_t));
		memcpy(checkKey, key->key, NNOB_KEY_BYTES);
		memxor(checkKey, npd->globalDelta, NNOB_KEY_BYTES);
		hashMacCheck(npd->gen, &npd->nonce, &npd->cumulativeHashCheckKey, checkKey);
		free(checkKey);
	}
	else {
		hashMacCheck(npd->gen, &npd->nonce, &npd->cumulativeHashCheckKey, &key->key);
	}
}

void getRandomAOTQuadrupleShareAndMacOfZ(NnobProtocolDesc* npd, 
		NnobKey* x0, NnobKey* x1, NnobShareAndMac* c, NnobShareAndMac* z) {
	int counter = npd->FDeal.aOTShareAndMacOfZ.counter;
	assert(counter<npd->FDeal.aOTShareAndMacOfZ.n);
	npd->FDeal.aOTShareAndMacOfZ.counter+=1;
	*x0=npd->FDeal.aOTShareAndMacOfZ.x0[counter];
	*x1=npd->FDeal.aOTShareAndMacOfZ.x1[counter];
	*c=npd->FDeal.aOTShareAndMacOfZ.c[counter];
	*z=npd->FDeal.aOTShareAndMacOfZ.z[counter];
}

void getRandomAOTQuadrupleKeyOfZ(NnobProtocolDesc* npd, 
		NnobShareAndMac* x0, NnobShareAndMac* x1, NnobKey* c, NnobKey* z) {
	int counter = npd->FDeal.aOTKeyOfZ.counter;
	assert(counter<npd->FDeal.aOTKeyOfZ.n);
	npd->FDeal.aOTKeyOfZ.counter+=1;
	*x0=npd->FDeal.aOTKeyOfZ.x0[counter];
	*x1=npd->FDeal.aOTKeyOfZ.x1[counter];
	*c=npd->FDeal.aOTKeyOfZ.c[counter];
	*z=npd->FDeal.aOTKeyOfZ.z[counter];
}

void getRandomAOTQuadruple(NnobProtocolDesc* npd, 
		OblivBit* x0, OblivBit* x1, OblivBit* c, OblivBit* z) {
	getRandomAOTQuadrupleShareAndMacOfZ(npd, &x0->nnob.key, &x1->nnob.key,
			&c->nnob.shareAndMac, &z->nnob.shareAndMac);	
	getRandomAOTQuadrupleKeyOfZ(npd, &x0->nnob.shareAndMac, &x1->nnob.shareAndMac,
			&c->nnob.key, &z->nnob.key);	
	fprintf(stderr, "aOT Quadruple\n");
}

void getRandomAANDTripleShareAndMac(NnobProtocolDesc* npd, 
		NnobShareAndMac* x, NnobShareAndMac* y, NnobShareAndMac* z) {
	int counter = npd->FDeal.aANDShareAndMac.counter;
	assert(counter<npd->FDeal.aANDShareAndMac.n);
	npd->FDeal.aANDShareAndMac.counter+=1;
	*x=npd->FDeal.aANDShareAndMac.x[counter];
	*y=npd->FDeal.aANDShareAndMac.y[counter];
	*z=npd->FDeal.aANDShareAndMac.z[counter];
}

void getRandomAANDTripleKey(NnobProtocolDesc* npd, 
		NnobKey* x, NnobKey* y, NnobKey* z) {
	int counter = npd->FDeal.aANDKey.counter;
	assert(counter<npd->FDeal.aANDKey.n);
	npd->FDeal.aANDKey.counter+=1;
	*x=npd->FDeal.aANDKey.x[counter];
	*y=npd->FDeal.aANDKey.y[counter];
	*z=npd->FDeal.aANDKey.z[counter];
}

void getRandomAANDTriple(NnobProtocolDesc* npd,
		OblivBit* x, OblivBit* y, OblivBit* z) {
	getRandomAANDTripleShareAndMac(npd, &x->nnob.shareAndMac, &y->nnob.shareAndMac,
			&z->nnob.shareAndMac);		
	getRandomAANDTripleKey(npd, &x->nnob.key, &y->nnob.key,
			&z->nnob.key);		
	fprintf(stderr, "aAND Tripple\n");
	debugPrintOblivBit(x);
	nl();
	debugPrintOblivBit(y);
	nl();
	debugPrintOblivBit(y);
	nl();
}

void nnobANDGetXYShareAndMac(ProtocolDesc* pd,  
		NnobShareAndMac* xy, const NnobShareAndMac* x, const NnobShareAndMac* y) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int destparty=pd->thisParty==1?1:2;
	
	NnobShareAndMac u,v,w,f,g;
	getRandomAANDTripleShareAndMac(npd, &u, &v, &w);
	f=u;
	nnobShareAndMacXOR(&f, x);
	g=v;
	nnobShareAndMacXOR(&g, y);
	osend(pd, destparty, &f.share, 1);	
	osend(pd, destparty, &g.share, 1);	
	hashMacCheckShareAndMac(npd, &f);
	hashMacCheckShareAndMac(npd, &g);
	*xy = w;
	if(f.share) nnobShareAndMacXOR(xy, y);
	if(g.share) nnobShareAndMacXOR(xy, x);
	if(f.share&&g.share) nnobShareAndMacXORConst(xy, true);
}

void nnobANDGetXYKey(ProtocolDesc* pd,  
		NnobKey* xy, const NnobKey* x, const NnobKey* y) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	int destparty=pd->thisParty==1?1:2;

	NnobKey u,v,w,f,g;
	bool other_f, other_g;
	getRandomAANDTripleKey(npd, &u, &v, &w);
	f=u;
	nnobKeyXOR(&f, x);
	g=v;	
	nnobKeyXOR(&g, y);
	orecv(pd, destparty, &other_f, 1);	
	orecv(pd, destparty, &other_g, 1);	
	hashMacCheckKey(npd, &f, other_f);
	hashMacCheckKey(npd, &g, other_g);
	*xy = w;
	if(other_f) nnobKeyXOR(xy, y);
	if(other_g) nnobKeyXOR(xy, x);
	if(other_f&&other_g) nnobKeyXORConst(xy, true, npd->globalDelta);
}

void nnobANDGetSKey(ProtocolDesc* pd, 
		NnobKey* s, NnobShareAndMac* r, const NnobShareAndMac* x, const NnobKey* y)
{
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	NnobShareAndMac u0, u1, f, g;
	bool other_d;
	NnobKey c, w, d;
	int destparty = pd->thisParty==1?1:2;
	getRandomAOTQuadrupleKeyOfZ(npd, &u0, &u1, &c, &w);
	d=c;	
	nnobKeyXOR(&d, y);
	orecv(pd, destparty, &other_d, 1);
	hashMacCheckKey(npd, &d, other_d);

	f=u0;
	nnobShareAndMacXOR(&f, &u1);
	nnobShareAndMacXOR(&f, x);
	g=u0;
	nnobShareAndMacXOR(&g, r);
	if(other_d) nnobShareAndMacXOR(&g, x);
	osend(pd, destparty, &f.share, 1);
	osend(pd, destparty, &g.share, 1);
	hashMacCheckShareAndMac(npd, &f);
	hashMacCheckShareAndMac(npd, &g);

	*s=w;
	if(f.share) nnobKeyXOR(s, &c);
	nnobKeyXORConst(s, g.share, npd->globalDelta);
}

void nnobANDGetSShareAndMac(ProtocolDesc* pd, 
		NnobShareAndMac* s, NnobKey* r, const NnobKey* x, const NnobShareAndMac* y)
{
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);
	NnobKey u0, u1, f, g;
	NnobShareAndMac c, w, d; 
	bool other_f, other_g;
	int destparty = pd->thisParty==1?1:2;
	getRandomAOTQuadrupleShareAndMacOfZ(npd, &u0, &u1, &c, &w);
	d=c;
	nnobShareAndMacXOR(&d, y);
	osend(pd, destparty, &d.share, 1);
	hashMacCheckShareAndMac(npd, &d);
	f=u0;
	nnobKeyXOR(&f, &u1);
	nnobKeyXOR(&f, x);
	g=*r;
	nnobKeyXOR(&g, &u0);
	if(d.share) nnobKeyXOR(&g, x);
	orecv(pd, destparty, &other_f, 1);
	orecv(pd, destparty, &other_g, 1);
	hashMacCheckKey(npd, &f, other_f);
	hashMacCheckKey(npd, &g, other_g);

	*s=w;
	if(other_f) nnobShareAndMacXOR(s, &c);
	nnobShareAndMacXORConst(s, other_g);
}

void nnobAND(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit *y) {
	NnobProtocolDesc* npd = pd->extra;
	assert(npd!=NULL);

	OblivBit r,xy,s;
	randomOblivAuthentication(npd, &r);

	if(pd->thisParty==1) {
		nnobANDGetXYShareAndMac(pd, &xy.nnob.shareAndMac, &x->nnob.shareAndMac, 
				&y->nnob.shareAndMac);
		nnobANDGetSKey(pd, &s.nnob.key, &r.nnob.shareAndMac, &x->nnob.shareAndMac, 
				&y->nnob.key);
		nnobANDGetXYKey(pd, &xy.nnob.key, &x->nnob.key, &y->nnob.key);
		nnobANDGetSShareAndMac(pd,&s.nnob.shareAndMac, &r.nnob.key, &x->nnob.key, 
				&y->nnob.shareAndMac);
	}
	else {
		nnobANDGetXYKey(pd, &xy.nnob.key, &x->nnob.key, &y->nnob.key);
		nnobANDGetSShareAndMac(pd,&s.nnob.shareAndMac, &r.nnob.key, &x->nnob.key, 
				&y->nnob.shareAndMac);
		nnobANDGetXYShareAndMac(pd, &xy.nnob.shareAndMac, &x->nnob.shareAndMac, 
				&y->nnob.shareAndMac);
		nnobANDGetSKey(pd, &s.nnob.key, &r.nnob.shareAndMac, &x->nnob.shareAndMac, 
				&y->nnob.key);
	}
	OblivBit temp;
	nnobXOR(&temp, &s, &xy);
	nnobXOR(z, &r, &temp);
}


void nnobSetBitAnd(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y ) {
	/*fprintf(stderr, "AND\n");*/
	nnobAND(pd, z, x, y);
	z->unknown=true;
	fprintf(stderr, "AND party %d\n", pd->thisParty);
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
	fprintf(stderr, "y=");
	debugPrintOblivBit(y);
	nl();
	fprintf(stderr, "z=");
	debugPrintOblivBit(z);
	nl();
}

void nnobSetBitXor(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y ) {
	/*fprintf(stderr, "XOR\n");*/
	nnobXOR(z, x, y);
	z->unknown=true;
	fprintf(stderr, "XOR party %d\n", pd->thisParty);
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
	fprintf(stderr, "y=");
	debugPrintOblivBit(y);
	nl();
	fprintf(stderr, "z=");
	debugPrintOblivBit(z);
	nl();
}

void nnobSetBitNot(ProtocolDesc* pd, OblivBit* z, const OblivBit *x) {
	/*fprintf(stderr, "NOT\n");*/
		
	OblivBit newZ = *x;
	nnobFlipBit(pd, &newZ);
	*z=newZ;
	z->unknown=true;
	fprintf(stderr, "NOT party %d\n", pd->thisParty);
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
	fprintf(stderr, "z=");
	debugPrintOblivBit(z);
	nl();
}

void nnobFlipBit(ProtocolDesc* pd, OblivBit* x) {
	fprintf(stderr, "FLIP party %d\n", pd->thisParty);
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
	if(pd->thisParty==1) {
		x->nnob.shareAndMac.share ^= 1;
	}
	else {
		const char* globalDelta = ((NnobProtocolDesc*)(pd->extra))->globalDelta;
		nnobKeyXORConst(&x->nnob.key,1, globalDelta);
	}
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
}

void nnobSetBitOr(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y ) {
	OblivBit xANDy;
	OblivBit xXORy;
	nnobSetBitXor(pd, &xXORy, x, y);
	nnobSetBitAnd(pd, &xANDy, x, y);
	nnobSetBitXor(pd, z, &xXORy, &xANDy);
	z->unknown=true;
	/*OblivBit notX;*/
	/*OblivBit notY;*/
	/*OblivBit notXandnotY;*/
	/*nnobSetBitNot(pd, &notX, x);*/
	/*nnobSetBitNot(pd, &notY, y);*/
	/*nnobSetBitAnd(pd, &notXandnotY, &notX, &notY);*/
	/*nnobSetBitNot(pd, z, &notXandnotY);*/
	fprintf(stderr, "OR party %d\n", pd->thisParty);
	fprintf(stderr, "x=");
	debugPrintOblivBit(x);
	nl();
	fprintf(stderr, "y=");
	debugPrintOblivBit(y);
	nl();
	fprintf(stderr, "z=");
	debugPrintOblivBit(z);
	nl();
}


void nnobSendBitOblivInputs(ProtocolDesc* pd, OblivBit* oblivInput, bool b, int destparty)
{
	NnobProtocolDesc* npd = pd->extra;
	bool sendbit;
	char const* globalDelta = npd->globalDelta;
	randomOblivAuthenticationShareAndMac(npd, &oblivInput->nnob.shareAndMac);	
	sendbit = b^oblivInput->nnob.shareAndMac.share;
	osend(pd, destparty, &sendbit, 1);
	memset(oblivInput->nnob.key.key, 0, NNOB_KEY_BYTES);
	nnobKeyXORConst(&oblivInput->nnob.key, sendbit, globalDelta);
	oblivInput->unknown=true;
	debugPrintOblivBit(oblivInput);
	nl();
}

void nnobRecvBitOblivInputs(ProtocolDesc* pd, OblivBit* oblivInput, int destparty)
{
	NnobProtocolDesc* npd = pd->extra;
	randomOblivAuthenticationKey(npd, &oblivInput->nnob.key);	
	memset(oblivInput->nnob.shareAndMac.mac, 0, NNOB_KEY_BYTES);
	orecv(pd, destparty, &oblivInput->nnob.shareAndMac.share, 1);
	oblivInput->unknown=true;
	debugPrintOblivBit(oblivInput);
	nl();
}

void nnobSendOblivInputs(ProtocolDesc* pd, OblivInputs* oi, int destparty) {
	int i;
	for(i=0;i<oi->size;i++) {
		bool b = getBit((const char*)&oi->src, i);
		nnobSendBitOblivInputs(pd, &(oi->dest)[i], b, destparty);
	}
}

void nnobRecvOblivInputs(ProtocolDesc* pd, OblivInputs* oi, int destparty) {
	int i;
	for(i=0;i<oi->size;i++) {
		nnobRecvBitOblivInputs(pd, &(oi->dest)[i], destparty);
	}
}

void nnobFeedOblivInputs(ProtocolDesc* pd ,OblivInputs* oi, size_t n, int src) {
	/*fprintf(stderr, "nnobFeedOblivInputs\n");*/
	int i;
	if(pd->thisParty==src) {
		for(i=0;i<n;i++) {
			nnobSendOblivInputs(pd, &oi[i], src==1?2:1);
		}
	}
	else
		for(i=0;i<n;i++) {
			nnobRecvOblivInputs(pd, &oi[i], src==1?2:1);
		}
}

void nnobRevealOblivInputsRecv(ProtocolDesc* pd, widest_t* dest,const OblivBit* o,size_t n, bool* share ,int destparty) {
	NnobProtocolDesc* npd = pd->extra;
	int i;
	fprintf(stderr, "A\n");
	orecv(pd, destparty, share, n);
	for(i=0;i<n;i++) {
		hashMacCheckKey(npd, &o[i].nnob.key, share[i]);
		dest[i] = share[i]^o[i].nnob.shareAndMac.share;
		fprintf(stderr, "%d\n", (int)dest[i]);
	}
	fprintf(stderr, "error? = %d\n", npd->error);
	npd->error |= !ocEqualityCheck(pd, npd->gen, npd->cumulativeHashCheckKey,NNOB_KEY_BYTES,destparty);
	fprintf(stderr, "error? = %d\n", npd->error);
	npd->nonce+=1;
	assert(!npd->error);
	fprintf(stderr, "in nnobRevealOblivInputs\n");
}

void nnobRevealOblivInputsSend(ProtocolDesc* pd, widest_t* dest,const OblivBit* o,size_t n, bool* share ,int destparty) {
	NnobProtocolDesc* npd = pd->extra;
	int i;
	fprintf(stderr, "B\n");
	for(i=0;i<n;i++) {
		share[i]=o[i].nnob.shareAndMac.share;
		hashMacCheckShareAndMac(npd, &o[i].nnob.shareAndMac);
	}
	osend(pd, destparty, share, n);
	fprintf(stderr, "error? = %d\n", npd->error);
	npd->error |= !ocEqualityCheck(pd, npd->gen, npd->cumulativeHashCheckMac,NNOB_KEY_BYTES, destparty);
	fprintf(stderr, "error? = %d\n", npd->error);
	npd->nonce+=1;
	assert(!npd->error);
	fprintf(stderr, "in nnobRevealOblivInputs\n");
}

bool nnobRevealOblivInputs(ProtocolDesc* pd, widest_t* dest,const OblivBit* o,size_t n,int party) {
	bool* share = malloc(n);
	fprintf(stderr, "n=%d\n", (int)n);
	fprintf(stderr, "party=%d\n", party);
	fprintf(stderr, "thisParty=%d\n", pd->thisParty);
	if(party==0) {
		if(pd->thisParty==1) {
			nnobRevealOblivInputsRecv(pd, dest, o, n, share, 3-pd->thisParty);
			nnobRevealOblivInputsSend(pd, dest, o, n, share, 3-pd->thisParty);
		}
		else {
			nnobRevealOblivInputsSend(pd, dest, o, n, share, 3-pd->thisParty);
			nnobRevealOblivInputsRecv(pd, dest, o, n, share, 3-pd->thisParty);
		}
	}
	else {
		if(party==pd->thisParty) {	
			nnobRevealOblivInputsRecv(pd, dest, o, n, share, 3-pd->thisParty);
		}
		else {
			nnobRevealOblivInputsSend(pd, dest, o, n, share, 3-pd->thisParty);
		}
	}
	fprintf(stderr, "Done!\n");

	free(share);
	return true;
}

#endif
