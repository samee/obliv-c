#ifndef NNOB_H
#define NNOB_H

#define BUCKET_SIZE 4
#define OTS_PER_aOT 4
#define OTS_PER_aAND 3 
#define OTS_FOR_R 1 
#define OTS_FOR_INPUTS 1 
#define NNOB_HASH_ALGO GCRY_CIPHER_AES192
#define NNOB_HASH_ALGO_KEYBYTES 24
//#define ALL_OTS (2*(OTS_PER_aOT+OTS_PER_aAND+OTS_FOR_R+OTS_FOR_INPUTS)) // 2* because both sides need 1
#define A_BIT_PARAMETER_BYTES (A_BIT_MULTIPLIER*NNOB_KEY_BYTES) 
#define A_BIT_MULTIPLIER 8 

typedef enum { OTExtValidation_hhash, OTExtValidation_byPair } OTExtValidation;
typedef enum {ShareAndMac=true, Key=false} NnobHalfBitType;

typedef struct {
	struct timespec wallclockTime;
	clock_t cpuclockTime;
} time_struct;

typedef struct AOTShareAndMacOfZ {
	NnobKey* x0;
	NnobKey* x1;
	NnobShareAndMac* c;
	NnobShareAndMac* z;
	int counter;
	int n;
} AOTShareAndMacOfZ;

typedef struct AOTKeyOfZ {
	NnobShareAndMac* x0;
	NnobShareAndMac* x1;
	NnobKey* c;
	NnobKey* z;
	int counter;
	int n;
} AOTKeyOfZ;

typedef struct AANDShareAndMac{
	NnobShareAndMac* x;
	NnobShareAndMac* y;
	NnobShareAndMac* z;
	int counter;
	int n;
} AANDShareAndMac;

typedef struct AANDKey{
	NnobKey* x;
	NnobKey* y;
	NnobKey* z;
	int counter;
	int n;
} AANDKey;

typedef struct NnobProtocolDesc
{
	bool error;
	int bucketSize, numANDs;
	nnob_key_t globalDelta;
	nnob_key_t cumulativeHashCheckKey;
	nnob_key_t cumulativeHashCheckMac;
	BCipherRandomGen* gen;
	int nonce;
	struct { // Also used to generate OTs
		bool* share;
		char (*mac)[NNOB_KEY_BYTES]; // equivalent to c bits
		int counter;
		int n;
	} aBitsShareAndMac;
	struct { // Also used to generate OTs
		char (*key)[NNOB_KEY_BYTES]; // equivalent to opt0
		int counter;
		int n;
	} aBitsKey;
	struct {
		AOTShareAndMacOfZ aOTShareAndMacOfZ;			
		AOTKeyOfZ aOTKeyOfZ;
		AANDShareAndMac aANDShareAndMac;
		AANDKey aANDKey;
	} FDeal;
} NnobProtocolDesc;

//void execNnobProtocol(ProtocolDesc* pd, protocol_run start, void* arg, int numOTs, bool useAltOTExt);
void nnobSetBitAnd(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y );
void nnobSetBitXor(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y );
void nnobSetBitNot(ProtocolDesc* pd, OblivBit* z, const OblivBit *x); 
void nnobFlipBit(ProtocolDesc* pd, OblivBit* x);
void nnobSetBitOr(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit*y );
void nnobFeedOblivInputs(ProtocolDesc* pd ,OblivInputs* oi, size_t n, int src);
bool nnobRevealOblivInputs(ProtocolDesc* pd, widest_t* dest,const OblivBit* o,size_t n,int party);
void execNnobProtocol(ProtocolDesc* pd, protocol_run start, void* arg, int numOTs, bool useAltOTExt);
void setupFDeal(NnobProtocolDesc* npd, int numOTs);
bool WaBitBoxGetBitAndMac(ProtocolDesc* pd, bool* b,
		char* mat, char (*aBitFullMac)[A_BIT_PARAMETER_BYTES],
		int n, OTExtValidation validation, int destparty);
bool WaBitBoxGetKey(ProtocolDesc* pd, nnob_key_t globalDelta,
		char* mat, char (*aBitFullKey)[A_BIT_PARAMETER_BYTES],
		int n, OTExtValidation validation, int destparty);
void WaBitToaBit(char (*aBit)[NNOB_KEY_BYTES], char (*WaBit)[A_BIT_PARAMETER_BYTES], char* mat, int n);
bool aOTKeyOfZ(ProtocolDesc *pd, AOTKeyOfZ* key);
bool aANDShareAndMac(ProtocolDesc *pd, AANDShareAndMac* sm);
bool aANDKey(ProtocolDesc *pd, AANDKey* key);
bool aOTShareAndMacOfZ(ProtocolDesc *pd, AOTShareAndMacOfZ* sm);

// debugger
void nnobAND(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit *y);
bool nnobRevealOblivBit(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* output, 
		OblivBit* input);
void setupNnobProtocol(ProtocolDesc* pd);
void mainNnobProtocol(ProtocolDesc* pd, int numOTs, OTExtValidation validation, protocol_run start, void* arg);
void cleanupNnobProtocol(ProtocolDesc* pd);

void debugNnobSendOblivInput(ProtocolDesc* pd, bool* input, OblivBit* oblivInput, 
		int numOblivInput);
void debugNnobRecvOblivInput(ProtocolDesc* pd, OblivBit* oblivInput, int numOblivInput);
void debugTimer(time_struct* t);
void debugPrintTime(time_struct* begin, time_struct* end, char* name, int party);
bool debugMatchingOblivBit(const OblivBit* x, const nnob_key_t globalDeltaToXKey,
		const OblivBit* y, const nnob_key_t globalDeltaToYKey);
bool debugMatchinKeyShareMac(const NnobShareAndMac* sm, const NnobKey* k, 
		const char* globalDelta);
//static int* allRows(int n);
void debugPrintOblivBit(const OblivBit* bit);
void nl();
void print(const char* a);
void printWithlen(const char* a, int len);
void debugPrintHex(const char* a, int len);
void debugPrintXor(const char* a,const  char* b, int len);
void debugPrintXorShowParam(const char* a,const  char* b, int len);

#endif
