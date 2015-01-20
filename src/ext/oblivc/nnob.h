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
#define A_BIT_PARAMETER_BYTES (A_BIT_MULTIPLIER*NNOB_KEY_BYTES) // NNOB_KEY_BYTES = 2*k
#define A_BIT_MULTIPLIER 8 

typedef enum { OTExtValidation_hhash, OTExtValidation_byPair } OTExtValidation;
typedef enum {ShareAndMac=true, Key=false} NnobHalfBitType;

typedef struct {
	struct timespec wallclockTime;
	clock_t cpuclockTime;
} time_struct;

typedef struct {
	NnobHalfBitType type;
	union {
		struct {
			bool value;
			nnob_key_t mac;
		} ShareAndMac;
		nnob_key_t key;
	};
} NnobHalfBit;

typedef struct NnobProtocolDesc
{
	bool error;
        int bucketSize;
	nnob_key_t globalDelta;
	int numANDs;
	struct { // Also used to generate OTs
		bool* share;
		char* mac; // equivalent to c bits
		int counter;
		int n;
	} aBitsShareAndMac;
	struct { // Also used to generate OTs
		char* key; // equivalent to opt0
		int counter;
		int n;
	} aBitsKey;
	struct {
		struct {
			struct {
				NnobHalfBit* x0;
				NnobHalfBit* x1;
				NnobHalfBit* c;
				NnobHalfBit* z;
				int counter;
				int n;
			} aOTShareAndMacOfZ;
			struct {
				NnobHalfBit* x0;
				NnobHalfBit* x1;
				NnobHalfBit* c;
				NnobHalfBit* z;
				int counter;
				int n;
			} aOTKeyOfZ;
		} aOTQuadruple;
		struct {
			struct {
				NnobHalfBit* x;
				NnobHalfBit* y;
				NnobHalfBit* z;
				int counter;
				int n;
			} aANDShareAndMac;
			struct {
				NnobHalfBit* x;
				NnobHalfBit* y;
				NnobHalfBit* z;
				int counter;
				int n;
			} aANDKey;
		} aANDTriple;
	} FDeal;
} NnobProtocolDesc;

void debugTimer(time_struct* t);
void debugPrintTime(time_struct* begin, time_struct* end, char* name, int party);
bool debugMatchingOblivBit(const OblivBit* x, const nnob_key_t globalDeltaToXKey,
		const OblivBit* y, const nnob_key_t globalDeltaToYKey);
void nnobGetOblivInput(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* input,
		OblivBit* x, OblivBit* y, int numOblivInput);
void nnobGetOblivInputSendShare(ProtocolDesc* pd, NnobProtocolDesc* npd, bool input,
		OblivBit* myInput);
void nnobGetOblivInputRecvShare(ProtocolDesc* pd, NnobProtocolDesc* npd, OblivBit* yourInput);
bool nnobRevealOblivBit(ProtocolDesc* pd, NnobProtocolDesc* npd, bool* output, 
		OblivBit* input);
static int* allRows(int n);
void randomOblivAuthentication(NnobProtocolDesc* pd, NnobHalfBit* bit, NnobHalfBitType type);
bool nnobSetBitAnd(ProtocolDesc* pd, OblivBit* z, const OblivBit *x, const OblivBit *y);
void debugPrintOblivBit(const OblivBit* bit);

void cleanupNnobProtocol(NnobProtocolDesc* npd);
NnobProtocolDesc* initNnobProtocolDesc(ProtocolDesc* pd, int numOTs, OTExtValidation validation,
		int destparty);

bool WaBitBoxGetBitAndMac(ProtocolDesc* pd, bool* b,
		char* mat, char* aBitFullMac,
		int n, OTExtValidation validation, int destparty);
bool WaBitBoxGetKey(ProtocolDesc* pd, nnob_key_t globalDelta,
		char* mat, char* aBitFullKey,
		int n, OTExtValidation validation, int destparty);
void WaBitToaBit(char* aBit, char* WaBit, char* mat, int n);

bool LaOT(ProtocolDesc* pd, NnobProtocolDesc* npd, const NnobHalfBit* x0, const NnobHalfBit* x1, const NnobHalfBit* c,
		const NnobHalfBit* r, NnobHalfBit* z,  
		BCipherRandomGen* padder, BCipherRandomGen* gen, int* nonce, int numLaOTs, int destparty);

void LaOTCombine(ProtocolDesc* pd,  
		NnobHalfBit* x10, const NnobHalfBit* x20, 
		NnobHalfBit* x11,const NnobHalfBit* x21, 
		NnobHalfBit* c1, const NnobHalfBit* c2, 
		NnobHalfBit* z1, const NnobHalfBit* z2, 
		int destparty);
bool aOT(ProtocolDesc* pd, NnobProtocolDesc* npd, NnobHalfBitType outputType, int bucketSize, int destparty);
bool LaAND(ProtocolDesc* pd, NnobProtocolDesc* npd, const NnobHalfBit* x, 
		const NnobHalfBit* y, const NnobHalfBit* r, NnobHalfBit* z, 
		BCipherRandomGen* padder, BCipherRandomGen* gen, int* nonce, int numLaANDs, int destparty);
void LaANDCombine(ProtocolDesc* pd, 
		NnobHalfBit* x1, const NnobHalfBit* x2, 
		NnobHalfBit* y1, const NnobHalfBit* y2,
		NnobHalfBit* z1, const NnobHalfBit* z2,
		int destparty);
bool aAND(ProtocolDesc *pd, NnobProtocolDesc* npd, NnobHalfBitType type, int bucketSize, int destparty);
void getRandomAOTQuadruple(NnobProtocolDesc* npd, 
		NnobHalfBit* x0, NnobHalfBit* x1, NnobHalfBit* c, NnobHalfBit* z, NnobHalfBitType zType);
void getRandomAANDTriple(NnobProtocolDesc* npd, 
		NnobHalfBit* x, NnobHalfBit* y, NnobHalfBit* z, NnobHalfBitType type);
void debugGetNnobHalfBit(NnobHalfBit* output, bool bit,const nnob_key_t key, 
		const nnob_key_t globalDelta, NnobHalfBitType type);
bool debugCheckOT(const NnobHalfBit* x0_1, const NnobHalfBit* x1_1,
		const NnobHalfBit* c_1, const NnobHalfBit* z_1, const nnob_key_t globalDelta_1,
		const NnobHalfBit* x0_2, const NnobHalfBit* x1_2,
		const NnobHalfBit* c_2, const NnobHalfBit* z_2, const nnob_key_t globalDelta_2);
bool debugCheckAND(const NnobHalfBit* x_1, const NnobHalfBit* y_1, 
		const NnobHalfBit* z_1, const nnob_key_t globalDelta_1,
		const NnobHalfBit* x_2, const NnobHalfBit* y_2,
		const NnobHalfBit* z_2, const nnob_key_t globalDelta_2);
void nl();
void print(const char* a);
void printWithlen(const char* a, int len);
void debugPrintHex(const char* a, int len);
void debugPrintXor(const char* a,const  char* b, int len);
void debugPrintXorShowParam(const char* a,const  char* b, int len);
void debugPrintNnobHalfBit(const NnobHalfBit* a);
bool debugMatchingNnobHalfBit(const NnobHalfBit* a, const NnobHalfBit* b, const nnob_key_t globalDelta);
void debugPrintKeyfromShareAndMac(const NnobHalfBit* a, const nnob_key_t globalDelta);
void debugPrintNnobHalfBitExtra(const NnobHalfBit* a, const nnob_key_t globalDelta);

#endif
