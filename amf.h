#ifndef __AMF_H
#define __AMF_H
#include <bits/stdc++.h>
#include <iostream>
#include <map>
#include <queue>
#include "platform/include/platform.h"
#include "ran/intf/include/ue.h"
#include "nas/mobility/RegistrationRequest.h"
#include "ngap/include/NGAP-PDU.h"

#define MAX_NUM_OF_STATES	(10)
#define MAX_NUM_OF_EVENTS	(20)

using namespace std;
using std::map;
using std::queue;

typedef enum {
	E_UE_INVALID_STATE = 0,
	E_UE_START_STATE = 1,          /* Just inited state */
	E_UE_REGISTRATION_STARTED,     /* Started Registration, sent
                                        * SUCI to AUSF.*/
	E_UE_AUTN_STARTED,             /* Started Authentication,
                                        * Sent RAND and AUTN to UE */
	E_UE_AMF_AUTH_COMPLETE,        /* AMF auth complete, * HXRES* == HRES*
                                        * waiting for (AUSF) home ntw auth*/
	E_UE_NAS_SECURITY_STARTED,     /* Started NAS Security procedure,
                                        * sent SECURITY MODE COMMAND */
	E_UE_NAS_SECURITY_COMPLETE,    /* NAS Security Done,
                                        * recvd SECURITY MODE COMPLETE */
	E_UE_MAX_STATE
} e_ue_fsm_states_t;

/*
 * struct MessageType and UserLocationInformation
 * should be part of messages dir.
*/
//struct MessageType {};
//struct UserLocationInformation {};

/* struct initialUEMessage {
	struct MessageType		messageType;
	uint32_t			ranueNgapid;
	uint8_t*			nasPdu;
	struct UserLocationInformation	userLocationInformation;
}; */

class amf {
	
	private:
		amf() {};
		~amf() {};
		static amf* mInstance;
	public:
		amf(amf const&)             = delete;
		void operator=(amf const&)  = delete;
		char snName[SERVING_NETWORK_NAME_LEN];

		typedef struct secContext {
					uint64_t K_NasInt;	/* these are 256 B values */
					uint64_t K_NasEnc;
					uint8_t ngKSI;
					uint8_t Kamf[32];
					_5gAv_t _5gAv;
					uint8_t direction;
					struct count_s{
						uint32_t spare:8;
						uint32_t overflow:16;
						uint32_t seq_num:8;
					} dl_count, ul_count;
					uint8_t   direction_encode; // SECU_DIRECTION_DOWNLINK, SECU_DIRECTION_UPLINK
					uint8_t   direction_decode; // SECU_DIRECTION_DOWNLINK, SECU_DIRECTION_UPLINK				

					secContext() { /* complete constr */
						this->K_NasInt=0;
						this->K_NasEnc=0;
						this->ngKSI=0;
						this->Kamf[32]= {};
						this->_5gAv={};
					}
				} secContext_t;

		typedef struct ueContext {
					RanUeNgapId_t ranUeNgapId;
					AmfUeNgapId_t amfUeNgapId;
					UeSecurityCapability_t ueSecurityCapability;
					//long ranUeNgapId;
					int currentFsmState; /* TODO e_ue_fsm_states_t */
					suci_t suci;
					supi_t supi;
					secContext_t secCtxt;
					ueContext() {
						this->currentFsmState = E_UE_INVALID_STATE;
						this->amfUeNgapId = 0;
						this->ranUeNgapId = 0;
						this->secCtxt._5gAv = {};
						this->secCtxt.dl_count = {};
						this->secCtxt.ul_count = {};
						this->secCtxt = {};
						this->suci = {};
						this->supi = {};
						this->ueSecurityCapability = {};
					}
				} ueContext_t;

		static amf* getInstance();
		//std::vector<ueContext_t *> connectedUe;
		map <AMF_UE_NGAP_ID_t, ueContext_t> ueMap;
		queue <AMF_UE_NGAP_ID_t> amfUeNgapIdFreeQ;
		void n1n2Fsm(NGAP_PDU_t *);
		void initN1N2Fsm();
		void init();
		void getSnName();

		int handleInitialUEMessage(void*);
		int handleUplinkNASTransport(void*);
		int handleRegistrationRequest(NAS_PDU_t*,ueContext_t&);
		int triggerAuthenticationRequest(ueContext_t&);
		int handleAuthenticationResponse(NAS_PDU_t*, ueContext_t&);
		int triggerAuthenticationResult(ueContext_t&, void*);
		int triggerSecurityModeCommand(ueContext_t&);
		int handleSecurityModeComplete(void *);
		int triggerInitialContextSetupRequest(ueContext_t &);
		int handleInitialContextSetupResponse(void*);
		int triggerRegistrationAccept();
		int authenticastionRequest();
		int handleRegistrationComplete(void*);
		void computeNgKSI(uint8_t ngKSI);
		void retrieveSuci(void *nasPdu, ueContext_t& ueContext);

};

typedef int (amf::*elemetaryProcedure)(void*);

#endif //__AMF_H
