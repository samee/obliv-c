#include <obliv_common.h>
#include <obliv_bits.h>
#include <commitReveal.h>
#include <nnob.h>
#include <assert.h>
#include <errno.h>      // libgcrypt needs ENOMEM definition
#include <inttypes.h>
#include <stdio.h>      // for protoUseStdio()
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <gcrypt.h>


#ifndef CURRENT_PROTO
#define CURRENT_PROTO
// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;
static inline bool known(const OblivBit* o) { return !o->unknown; }
#endif

//--------------------------- NNOB Protocol ---------------------------------
#ifdef ENABLE_NNOB

/*void nnobAndGatesCount(ProtocolDesc* pd, protocol_run start, void* arg)*/
/*{*/
  /*pd->currentParty = ocCurrentPartyDefault;*/
  /*pd->error = 0;*/
  /*pd->feedOblivInputs = dbgProtoFeedOblivInputs;*/
  /*pd->revealOblivBits = dbgProtoRevealOblivBits;*/
  /*pd->setBitAnd = dbgProtoSetBitAnd;*/
  /*pd->setBitOr  = dbgProtoSetBitOr;*/
  /*pd->setBitXor = dbgProtoSetBitXor;*/
  /*pd->setBitNot = dbgProtoSetBitNot;*/
  /*pd->flipBit   = dbgProtoFlipBit;*/
  /*pd->partyCount= 2;*/
  /*pd->extra = NULL;*/
  /*currentProto = pd;*/
  /*currentProto->debug.mulCount = currentProto->debug.xorCount = 0;*/
  /*start(arg);*/
/*}*/

void setupNnobProtocol(ProtocolDesc* pd) {
	NnobProtocolDesc* npd = malloc(sizeof(NnobProtocolDesc));
	pd->extra=npd;
	pd->error = 0;
	pd->partyCount = 2;
	pd->currentParty = ocCurrentPartyDefault;
	pd->feedOblivInputs = nnobFeedOblivInputs; 
	pd->revealOblivBits = nnobRevealOblivInputs; 
	pd->setBitAnd = nnobSetBitAnd;
	pd->setBitOr  = nnobSetBitOr; 
	pd->setBitXor = nnobSetBitXor;
	pd->setBitNot = nnobSetBitNot;
	pd->flipBit   = nnobFlipBit;
}

void mainNnobProtocol(ProtocolDesc* pd, int numOTs, OTExtValidation validation, protocol_run start, void* arg) {
	/*int denom = logfloor(numOTs, 2)+1;*/
	/*int bucketSize = (int)(1 + (NNOB_KEY_BYTES*8)/denom);*/
	int bucketSize = BUCKET_SIZE;
	NnobProtocolDesc* npd = pd->extra; 
	npd->bucketSize = bucketSize;
	int n = ((numOTs+7)/8)*8;
	int	destparty = pd->thisParty==1?1:2;
	memset(npd->cumulativeHashCheckKey, 0, NNOB_KEY_BYTES);
	memset(npd->cumulativeHashCheckMac, 0, NNOB_KEY_BYTES);
	char dummy[NNOB_HASH_ALGO_KEYBYTES];
	dhRandomInit();
	npd->gen= newBCipherRandomGenByAlgoKey(NNOB_HASH_ALGO, dummy);
	npd->nonce = 0;

	npd->aBitsShareAndMac.counter = 0;
	npd->aBitsShareAndMac.n = n;
	npd->aBitsShareAndMac.share = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsShareAndMac.mac = malloc(n*NNOB_KEY_BYTES);

	npd->aBitsKey.key = malloc(n*NNOB_KEY_BYTES);
	npd->aBitsKey.counter = 0;
	npd->aBitsKey.n = n;

	setupFDeal(npd, numOTs);

	npd->error = false;

	char mat1[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char mat2[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
	char (*aBitFullMac)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	char (*aBitFullKey)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
	if(destparty==1)
	{
		npd->error |= !WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat1, aBitFullMac, numOTs, validation, destparty);
		npd->error |= !WaBitBoxGetKey(pd, npd->globalDelta, 
				mat2, aBitFullKey, numOTs, validation, destparty);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat1, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat2, numOTs);
		npd->error |= !aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
		npd->error |= !aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
		npd->error |= !aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
		npd->error |= !aANDKey(pd, &npd->FDeal.aANDKey);
	}
	else
	{
		npd->error |= !WaBitBoxGetKey(pd, npd->globalDelta, 
				mat1, aBitFullKey, numOTs, validation, destparty);
		npd->error |= !WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share, 
				mat2, aBitFullMac, numOTs, validation, destparty);
		WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat2, numOTs);
		WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat1, numOTs);
		npd->error |= !aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
		npd->error |= !aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
		npd->error |= !aANDKey(pd, &npd->FDeal.aANDKey);
		npd->error |= !aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
	}
	free(aBitFullKey);
	free(aBitFullMac);
	currentProto = pd;
	start(arg);
}

void cleanupNnobProtocol(ProtocolDesc* pd)
{
	NnobProtocolDesc* npd = pd->extra;
	releaseBCipherRandomGen(npd->gen);
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

void execNnobProtocol(ProtocolDesc* pd, protocol_run start, void* arg, int numOTs, bool useAltOTExt) {
	OTExtValidation validation = useAltOTExt?OTExtValidation_byPair:OTExtValidation_hhash;
	setupNnobProtocol(pd);
	mainNnobProtocol(pd, numOTs, validation, start, arg);

	NnobProtocolDesc* npd = pd->extra;
	int numANDGates = npd->FDeal.aANDKey.n;
	fprintf(stderr, "num of ANDs: %d\n", numANDGates);
	fprintf(stderr, "OTs per AND: %d\n", numOTs/numANDGates);
	fprintf(stderr, "OTs left: %d\n", npd->aBitsShareAndMac.n-npd->aBitsKey.counter);
	fprintf(stderr, "Bucket Size: %d\n", npd->bucketSize);
	cleanupNnobProtocol(pd);
}
#endif // ENABLE_NNOB

