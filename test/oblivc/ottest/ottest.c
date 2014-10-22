#include<stdio.h>
#include<obliv.h>
#include<obliv_common.h>

// If party 1: party_number, opt0, opt1, n, len
// If party 2: party_number, sel, n, len
int main(int argc, char* argv[])
{
	ProtocolDesc pd;
        dhRandomInit();
	protocolUseStdio(&pd);
	setCurrentParty(&pd,argv[1][0]=='1'?1:2);
	if(pd.thisParty==1)
	{
		char *opt0 = argv[2]; 
		char *opt1 = argv[3]; 
		int n = atoi(argv[4]);
		int len = atoi(argv[5]);
		struct HonestOTExtSender *sender = honestOTExtSenderNew(&pd, 2);
		honestOTExtSend1Of2(sender, opt0, opt1, n, len);
		honestOTExtSenderRelease(sender);
	}
	else 
	{
		int i;
		int n = atoi(argv[3]);
		bool *sel = malloc(n*sizeof(bool));
		for(i=0;i<n;i++) sel[i]=argv[2][i]-'0';
		int len = atoi(argv[4]);
		char *output = malloc(n*len);
		struct HonestOTExtRecver *recver = honestOTExtRecverNew(&pd, 1);
		honestOTExtRecv1Of2(recver, output, sel, n, len);
		honestOTExtRecverRelease(recver);
		fprintf(stderr, "%s\n", output);
		free(sel);
		free(output);
	}
	cleanupProtocol(&pd);
	return 0;
}
