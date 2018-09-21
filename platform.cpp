#include "platform.h"
#include "logging.h"
#include <bits/stdc++.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
using namespace std;
map<int,string> compId_to_nfvName;
struct nfvInstanceData *gNfvInst = NULL;	/* a global var, to access FDs*/

unsigned int gPortMap[E_MAX_NFV_COMPONENTS][E_MAX_NFV_COMPONENTS] = {
	{9000, 5100},
	{5100, 9000}
};

_e_socketType gSocketTypeMap[E_MAX_NFV_COMPONENTS][E_MAX_NFV_COMPONENTS] = {
	{SELF_CONNECT, SCTP_CLIENT_SOCKET},
	{SCTP_SERVER_SOCKET, SELF_CONNECT}
};
#define MAX_EPOLL_EVENTS	E_MAX_NFV_COMPONENTS*MAX_SCTP_CONNECTIONS
#define MAX_MESSAGE_SIZE	3000


char nfvCompIpAddr[E_MAX_NFV_COMPONENTS][IP_ADDR_MAX_STRLEN] = {
	{"192.168.122.10"},
	{"192.168.122.20"}
};

/* function to set NONBLOCK flag on socket
 * check it's output if it is a must have flag.
 * 0 for success, -1 for error
 */
int setNonBlock(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		higLog("fcntl get flags failed on fd %d, Error %s", fd,
				strerror(errno));
		return -1;
	}

	if(fcntl(fd, F_SETFL, flags|O_NONBLOCK) == -1) {
		higLog("fcntl set flags failed on fd %d, Error %s", fd,
				strerror(errno));
		return -1;
	}
	return 0;

}

/* Function to open and bind sockets.
 * only Non-Blocking sockets
 * returns -1 on failure, fd on success */
int openSocketAndInit(struct fdData *fdd, int port)
{
	int fd = 0;
	struct sockaddr_in myaddr = {};
	fdd->listenFD = -1;

	/*no connection necessary to itself*/
	if (fdd->type == SELF_CONNECT) {
		return 0;

	/*A UDP connection*/
	} else if (fdd->type == UDP_SOCKET) {
		fd = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK, 0);
		if (fd == -1) {
			higLog("UDP socket create failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}

		/*now bind to the port*/
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(port);
		//myaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//need it bound per addr?
		if (bind(fd, (struct sockaddr*) &myaddr, sizeof(myaddr))== -1){
			higLog("UDP bind failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}

		fdd->listenFD = fd;
		lowLog("UDP port %d, creation & bind successful", port);

	/*A SCTP client socket*/
	} else if (fdd->type == SCTP_CLIENT_SOCKET) {
		fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
		if (fd == -1) {
			higLog("SCTP CLIENT create failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}

		/*now connect to the port*/
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(port);
		myaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);	//need it bound per addr?

		/*allow connect some time, add SOCK_NONBLOCK after connect*/
		if (connect(fd, (struct sockaddr*)&myaddr,sizeof(myaddr))== -1){
			higLog("SCTP CLIENT connect failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}
		setNonBlock(fd);

		fdd->listenFD = fd;
		lowLog("SCTP CLIENT port %d, creation & connect successful",
				port);

	/* A SCTP server socket*/
	} else if (fdd->type == SCTP_SERVER_SOCKET) {
		fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
		if (fd == -1) {
			higLog("SCTP SERVER create failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(port);
		//myaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//need it bound per addr?
		int tr = 1;
		if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
			higLog("Port reuse failed, port %d, Error %s",
					port, strerror(errno));
    		return -1;
		}
		/*now bind to the port*/
		if (bind(fd, (struct sockaddr*) &myaddr, sizeof(myaddr))== -1){
			higLog("SCTP SERVER bind failed, port %d, Error %s",
					port, strerror(errno));
			return -1;
		}
		if(listen(fd, LISTEN_QUEUE_BACKLOG) == -1) {
			higLog("SCTP SERVER listen failed, port %d, Error %s",
					port, strerror(errno));
			close(fd);
			return -1;
		}
		fdd->listenFD = fd;
		fdd->dataFD = (int*) malloc(sizeof(int)*MAX_SCTP_CONNECTIONS);
		memset(fdd->dataFD, -1, (sizeof(int)*MAX_SCTP_CONNECTIONS));
		lowLog("SCTP SERVER port %d, creation & connect successful",
				port);

	} else {
		higLog("Unknown socket type %d", fdd->type);
		return -1;
	}
	return fd;
}

/*
 * This function opens socket on nfv-server for each channel.
 * The socket descriptor is then stored in fdList.
 *
 * Return: SUCCESS/FAILURE
*/
int openChannels(struct nfvInstanceData* nfvInst)
{
	unsigned int port = 0;
	int i = 0;
	compId_to_nfvName[0] = "AMF";
	compId_to_nfvName[1] = "RAN";
	for(i=0; i<(E_MAX_NFV_COMPONENTS); i++) {
		port = gPortMap[nfvInst->compId][i];
		nfvInst->fdData[i].type =  gSocketTypeMap[nfvInst->compId][i];

		openSocketAndInit(&nfvInst->fdData[i], port);
	}

	return SUCCESS;
}

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
struct nfvInstanceData* initPlatform(_e_nfv_component selfNfvCompId) {
	struct nfvInstanceData *nfvInst;

	nfvInst = (struct nfvInstanceData*)
				malloc(sizeof(struct nfvInstanceData));
	nfvInst->compId = (int)selfNfvCompId;
	strncpy(nfvInst->ipAddr, nfvCompIpAddr[nfvInst->compId], IP_ADDR_MAX_STRLEN-1);
	
	nfvInst->ipAddr


	openChannels(nfvInst);
	gNfvInst = nfvInst;

	return nfvInst;
};

int platformSendToPeer(_e_nfv_component peer, char *msg, int len,
						struct sockaddr *sin)
{
	struct fdData *fdd = &gNfvInst->fdData[peer];
	int rc = 0;

	switch(fdd->type) {
	case UDP_SOCKET:
		rc = sendto(fdd->listenFD, msg, len, 0, sin,
					sizeof(struct sockaddr_in));
		break;
	case SCTP_CLIENT_SOCKET:
		rc = sendto(fdd->listenFD, msg, len, 0, NULL, 0);
		break;
	case SCTP_SERVER_SOCKET:
		/* This is clearly a dirty hack. Will only work if at any point
		 * there is only a single ran fd.
		 * This is temporary. Will be replaced with after we come up
		 * with a cleaner design.
		 */
		rc = sendto(fdd->dataFD[0], msg, len, 0, NULL, 0);
		break;
	default:
		higLog("Incorrect peer ID or corrupted fdData struct. peer %s, socket type %d",
					peer, fdd->type);
	}
	if(rc == -1) {
		lowLog("sendto failed, Error: %s", strerror(errno));
		return rc;
	} else
		return rc;

}


/* send the message, return rc of sendto
 */
int platformSendData(int fd, char *msg, int len, struct sockaddr *sin)
{
	int rc;

	rc = sendto(fd, msg, len, 0, sin,
			sin ? sizeof(struct sockaddr_in) : 0);
	if(rc == -1) {
		lowLog("sendto failed, Error: %s", strerror(errno));
		return rc;
	} else
		return rc;
	
}

/* recv the message, return recvfrom's output */
int platformRecvData(int activeFD, char *msg)
{
	int rc;

	rc = recvfrom(activeFD, msg, MAX_MESSAGE_SIZE,
			0, NULL, NULL);
	if(rc == -1) {
		lowLog("recvfrom failed, Error: %s", strerror(errno));
		return rc;
	} else if(rc == 0) {
		lowLog("%s", "recvfrom returned 0 len msg");
		return rc;
	} else
		return rc;
}


/* accept connections and add it to epoll
 * return 1 if it was a listen socket, else 0 */
int acceptConnections(struct fdData *fdd, int epollFd, int activeFd)
{
	int channel, newfd, emptyFD;
	struct epoll_event event;

	for(channel=0; channel<E_MAX_NFV_COMPONENTS; channel++) {
		if(fdd[channel].listenFD == activeFd &&
		   fdd[channel].type == SCTP_SERVER_SOCKET) {
			newfd = accept4(activeFd, NULL, NULL, SOCK_NONBLOCK);
			if(newfd == -1) {
				higLog("accept4 failed, Error: %s",
							strerror(errno));
				return 1;
			}

			/*ok, add it to epoll*/
			memset(&event,0, sizeof(event));
			event.events = EPOLLIN;
			event.data.fd = newfd;
			if(epoll_ctl(epollFd, EPOLL_CTL_ADD, newfd,&event)==-1){
				higLog("epoll_ctl failed, nfv %d 's listenFD, Error %s",
						channel, strerror(errno));
				return 1;
			}

			for(emptyFD=0;emptyFD<MAX_SCTP_CONNECTIONS;emptyFD++){
				if(fdd->dataFD[emptyFD] == -1)
					break;
			}
			if(emptyFD == MAX_SCTP_CONNECTIONS) {
				higLog("SCTP SERVER channel %d, full", channel);
				return 0;
			}
			fdd->dataFD[emptyFD] = newfd;

			/*no more work*/
			return 1;
		}
	}
	return 0;
}

void removeFdFromEpoll(int fd, int epollFd)
{
	if(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL)==-1) {
		higLog("epoll_ctl DEL failed, Error %s", strerror(errno));
	}

	if(close(fd) == -1) {
		higLog("close failed, Error %s", strerror(errno));
	}

}

void recvAndProcessCallbacks(struct nfvInstanceData *nfvInst, int epollFd,
								int activeFd)
{
	int channel, rc, fd;
	struct fdData *fdd = nfvInst->fdData;
	int *dataFDs;
	char *msg;

	msg = (char *) malloc(sizeof(char)* MAX_MESSAGE_SIZE);
	/* alloc here, either freed in CR
	 * or MUST be freed in the current func (failure or CB is not called) */

	for(channel=0; channel<E_MAX_NFV_COMPONENTS; channel++) {
		/* is it one of the server data sockets ?*/
		if(fdd[channel].type == SCTP_SERVER_SOCKET) {
			dataFDs = fdd[channel].dataFD;
			for(fd = 0; fd<MAX_SCTP_CONNECTIONS; fd++) {
				if(dataFDs[fd] != activeFd)
					continue;
				rc = platformRecvData(activeFd, msg);
				if(rc > 0) {
					lowLog("Call cb on nfv %d:%s", channel,
							compId_to_nfvName[channel].c_str());
					/* call registered callback */
					nfvInst->CB[channel]((void*) msg, rc);
				} else if(rc == 0) {
					removeFdFromEpoll(activeFd, epollFd);
					midLog("Removed dataFD %d", fd);
					dataFDs[fd] = -1;
					free(msg);
				}
				return;
			}
		}

		if(fdd[channel].listenFD != activeFd)
			continue;

		if(fdd[channel].type == UDP_SOCKET) {
			rc = platformRecvData(activeFd, msg);
			if(rc > 0) {
				lowLog("Call cb on nfv %d", channel);
				//nfvInst->CB[channel]((void*) msg);
			} /*UDP: ignore if 0 length message is received */
			else
				free(msg);
			return;

		} else if(fdd[channel].type == SCTP_CLIENT_SOCKET) {
			rc = platformRecvData(activeFd, msg);
			lowLog("ENTRY\n");
			if(rc > 0) {
				lowLog("Call cb on nfv %d", channel);
				//nfvInst->CB[channel]((void*) msg);
			} else if(rc == 0) {
				removeFdFromEpoll(activeFd, epollFd);
				higLog("deleted SCTP client socket, nfv %d",
						channel);
				fdd[channel].listenFD = -1;
				free(msg);
			}
			return;
		}
	}
}


/* a blocked call. Will poll on sockets and run corresponding callbacks
 */
void pollOnEvents(struct nfvInstanceData *nfvInst)
{
	int epollFd, channel, numFds, fd;
	struct epoll_event event, activeEvents[MAX_EPOLL_EVENTS];
	struct fdData *fdd;

	/*create a epollfd*/
	epollFd = epoll_create1(0);
	if(epollFd == -1) {
		higLog("epoll fd creation failed, Error: %s", strerror(errno));
		exit(1);
	}

	/*Add all active FDs*/
	for(channel = 0; channel < E_MAX_NFV_COMPONENTS; channel++) {
		fdd = &nfvInst->fdData[channel];
		if(fdd->type == SELF_CONNECT || fdd->listenFD == -1)
			continue;	/*ignore theses FDs*/

		memset(&event,0, sizeof(event));
		event.events = EPOLLIN;
		event.data.fd = fdd->listenFD;
		if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fdd->listenFD,&event)==-1){
			higLog("epoll_ctl failed, nfv %d 's listenFD, Error %s",
					strerror(errno));
			continue;
		}
		/* dataFDs will added when they get created. */
	}

	/* epoll on fds, call corresponding callback */
	lowLog("%s","polling on fds, on rcving data, call correct callback");
	while(1) {
		lowLog("%s","..."); //TODO remove this.

		/* polling... */
		numFds = epoll_wait(epollFd, activeEvents, MAX_EPOLL_EVENTS, -1);
		higLog("Got %d events, processing.............................",
				numFds);
		for(fd = 0; fd < numFds; fd++) {
			/*1. on all listening stream sockets accept and
			 *   add the sock to epoll*/
			if(acceptConnections(nfvInst->fdData, epollFd,
						activeEvents[fd].data.fd))
				continue;

			/*2. recv and process callbacks on data fds */
			recvAndProcessCallbacks(nfvInst, epollFd,
						activeEvents[fd].data.fd);
		}
	}

	return;
}

