/* 
 * This file specifies set up of system under test(SUT)
 * It enumerates NFV components which form part of SUT.
 * It specifies the ip addresses of components.
 *
 * setup.h being a compile time entity, the SUT parameters
 * need to be specified at compile time. The rationale
 * behind this to speed up core development & avoid getting 
 * into secondary issues of reading configuration files
 * 
 * At a later stage set up can be made run time
*/

#ifndef __SETUP_H
#define __SETUP_H

#include "common/include/datatypes.h"

/* NFV components to be deployed in SUT */
typedef enum nfv_component {
	SEED_LIST_SERVER = 0,
	
	SEED_NODE_INSTANCE_0,
	SEED_NODE_INSTANCE_1,
	SEED_NODE_INSTANCE_2,
	SEED_NODE_INSTANCE_3,

	CLIENT_NODE_INSTANCE_0,
	CLIENT_NODE_INSTANCE_1,
	CLIENT_NODE_INSTANCE_2,
	CLIENT_NODE_INSTANCE_3,
	CLIENT_NODE_INSTANCE_4,
	CLIENT_NODE_INSTANCE_5,
	CLIENT_NODE_INSTANCE_6,
	CLIENT_NODE_INSTANCE_7,
	CLIENT_NODE_INSTANCE_8,
	CLIENT_NODE_INSTANCE_9,
	CLIENT_NODE_INSTANCE_10,
		
	// E_SMF_INST_1 -- next phase
	E_MAX_NFV_COMPONENTS
} _e_nfv_component;

extern char nfvCompIpAddr[E_MAX_NFV_COMPONENTS][IP_ADDR_MAX_STRLEN];


#endif //__SETUP_H
