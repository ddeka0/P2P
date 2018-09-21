#include "common/include/setup.h"
#include "platform/include/platform.h"
#include "platform/include/logging.h"
#include "amf/include/amf.h"
#include <stdlib.h>
#include <unistd.h>
#include "ngap/include/NGAP-PDU.h"
#include "ngap/codec/include/ngap_helper.h"

void amfHandleForLoadGeneratorMsgs(void *msgBlob, int len)
{
	NGAP_PDU_t *ngapPdu = NULL;
	
	if (decodeNgapPdu(&ngapPdu, msgBlob, len) == FAILURE) {
		higLog("%s", "Decode failed"); //TODO: redundant logging, rm
		//handle decode failure
		free(msgBlob);
		return;
	}

	(amf::getInstance())->n1n2Fsm(ngapPdu);

	ngapFree(ngapPdu); /* free decoded struct */
	free(msgBlob);
}

void registerCallbacks(struct nfvInstanceData* nfvInst)
{
	nfvInst->CB[E_LOAD_GENERATOR_INST_1] = amfHandleForLoadGeneratorMsgs;
	nfvInst->CB[E_AMF_INST_1] = 0;
}

struct nfvInstanceData* init(_e_nfv_component comp)
{
	struct nfvInstanceData *nfvInst;

	nfvInst = initPlatform(comp);
	registerCallbacks(nfvInst);
	return nfvInst;
}

void sigpipe_handler(int unused)
{
	midLog("%s", "received SIGPIPE signal. maybe the server died");
}

void sigint_handler(int unused)
{
	higLog("%s", "Received SIGINT, exiting");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	struct nfvInstanceData *amfNfvInst;

	signal(SIGPIPE, sigpipe_handler);
	//signal(SIGINT, sigint_handler);
	if(argc != 2) {
		
	}

	amfNfvInst = init(E_AMF_INST_1);
	
	pollOnEvents(amfNfvInst);

	return 0;
}
