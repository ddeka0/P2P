/*
 * This file contains datastructures pertaining to platform
 *
 * Platform acts a software interface which calls socket calls 
*/

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "common/include/datatypes.h"
#include "common/include/setup.h"

/* gFdList array maintains socket fds for all channels */
/*
 * Better not to keep this global. Encapsulate in platform.
 * Thay way only platform will have write access to fdList.
 *
 * Each process will form channels with all nfv comp instances.
 * Likewise gFdList has E_MAX_NFV_COMPONENTS channel fds.
*/
/*
 * Similarly ipAddr,compId,CB[] should be part of object.
 * Thiss object should be encapsulated at platform layer.
*/
typedef void (*cbs)(void * msg, int len);
#define MAX_SCTP_CONNECTIONS	10
#define MAX_EPOLL_EVENTS	E_MAX_NFV_COMPONENTS*MAX_SCTP_CONNECTIONS
#define MAX_MESSAGE_SIZE	3000
#define LISTEN_QUEUE_BACKLOG	10

typedef enum socketTypes {
	SELF_CONNECT = -1,
	UDP_SOCKET = 0,
	SCTP_SERVER_SOCKET,
	SCTP_CLIENT_SOCKET,
	NUM_SOCKET_TYPES	//always the last one
} _e_socketType;

struct fdData {
	_e_socketType type;	//UDP or SCTP
	int listenFD;
	int *dataFD;		//alloc it only for SCTP SERVER sockets
};


struct nfvInstanceData {
	int	compId;
	struct	fdData	fdData[E_MAX_NFV_COMPONENTS];
	char	ipAddr[IP_ADDR_MAX_STRLEN];
	cbs	CB[E_MAX_NFV_COMPONENTS];
};

/*
 * This function initializes platform objects. It also makes
 * function calls which open listening socket for each channel.
 * The number of channels depend on setup sepcified.
 *
 * Arg IN: selfNfvCompId stands for comp id of process which
 *	   is making use of Platform services. The process
 *	   which is running as nfv.
 *
 * Return: nfvInstanceData pointer.
*/
struct nfvInstanceData* initPlatform(_e_nfv_component selfNfvCompId);

/* a blocked call. Will poll on sockets and run corresponding callbacks
 */
void pollOnEvents(struct nfvInstanceData *nfvInst);
int platformSendData(int fd, char *msg, int len, struct sockaddr *sin);
int platformRecvData(int activeFD, char *msg);
int platformSendToPeer(_e_nfv_component peer, char *msg, int len,
						struct sockaddr *sin);

#endif //__PLATFORM_H
