#include <bits/stdc++.h>
#include "setup.h"
#include "platform.h"
#include "logging.h"
#include <stdlib.h>
#include <unistd.h>
void ranHandleForAmf(void *msgBlob, int len) {
	//(amf::getInstance())->entryPoint(msgBlob, E_LOAD_GENERATOR_INST_1);
	free(msgBlob);
}

void registerCallbacks(struct nfvInstanceData* nfvInst) {
	nfvInst->CB[E_LOAD_GENERATOR_INST_1] = 0;
	nfvInst->CB[E_AMF_INST_1] = ranHandleForAmf;
}

struct nfvInstanceData* init(_e_nfv_component comp) {
	struct nfvInstanceData *nfvInst;
	nfvInst = initPlatform(comp);
	registerCallbacks(nfvInst);
	return nfvInst;
}


/*  TODO send valid values, free calloc'ed structs  */
void testSendInitialUeMessage(struct fdData * fdd) {
	platformSendData(fdd->listenFD,msg, encRet.encoded, NULL);
	ngapFree(ngapPdu);
}



/* void testRecvAndPrintMessage(struct fdData *fdd) {
	char msg[MAX_MESSAGE_SIZE];
	NGAP_PDU_t  *ngapPdu = 0;
	asn_dec_rval_t decRet;
	int msgLen;

	if((msgLen = platformRecvData(fdd->listenFD, msg)) <= 0)
		return;

	// decode msg into NGAP PDU
	decRet = ber_decode(0, &asn_DEF_NGAP_PDU,
					(void**) &ngapPdu, msg, msgLen);
	if(decRet.code == RC_OK) {
		lowLog("%s", "Decode succeeded");
		asn_fprint(stdout, &asn_DEF_NGAP_PDU, ngapPdu);
		ASN_STRUCT_FREE(asn_DEF_NGAP_PDU, ngapPdu);
	} else
		higLog("Decode failed, Err %s", decRet.code == RC_FAIL ?
						"RC_FAIL" : "RC_WMORE");

} */


void sigpipe_handler(int unused) {
	midLog("%s", "received SIGPIPE signal. maybe the server died");
}
int main(int argc, char *argv[]) {
	struct nfvInstanceData *ranNfvInst;
	if(argc != 2) {
		
	}
	signal(SIGPIPE, sigpipe_handler);
	ranNfvInst = init(E_LOAD_GENERATOR_INST_1);
	return 0;
}

