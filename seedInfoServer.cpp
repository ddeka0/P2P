#include <bits/stdc++.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>
#include <iostream>
#include <string>
#include "logging.h"
#include "seedInfo.pb.h"
using namespace std;
#define MAX_BUFFER              3000
#define MY_PORT_NUM             54145
#define MAX_BACKLOG_REQUEST     100

int main () {
    int listenSock, connSock, ret, in, flags, i;
    struct sockaddr_in servaddr;
    struct sctp_initmsg initmsg;
    struct sctp_event_subscribe events;
    struct sctp_sndrcvinfo sndrcvinfo;
    char buffer[MAX_BUFFER + 1];
    int datalen = 0;

    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if(listenSock == -1) {
        higLog("%s","Failed to create socket");
        exit(1);
    }
    int opt = SO_REUSEADDR;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        higLog("%s","setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (MY_PORT_NUM);

    ret = bind (listenSock, (struct sockaddr *) &servaddr, sizeof (servaddr));

    if(ret == -1 ) {
        higLog("%s","bind() failed");
        close(listenSock);
        exit(1);
    }
    /* Specify that a maximum of 5 streams will be available per socket */
    memset (&initmsg, 0, sizeof (initmsg));
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    ret = setsockopt (listenSock, IPPROTO_SCTP, SCTP_INITMSG, 
            &initmsg, sizeof (initmsg));

    if(ret == -1 ) {
        higLog("%s","setsockopt() failed while setting no of streams");
        close(listenSock);
        exit(1);
    }
    ret = listen(listenSock, MAX_BACKLOG_REQUEST);
    if(ret == -1 ) {
        higLog("listen() failed \n");
        close(listenSock);
        exit(1);
    }
    // create the List of seedNode  < Name , IPs >
    SeedInfoServer::seedInfo SeedInfoList;
    auto& Map = *SeedInfoList.mutable_seedlist();
    Map["SeedServerA"] = "10.11.110.12";
    
    while(true) {
        char buffer[MAX_BUFFER + 1];
        int len;
        bzero (buffer, MAX_BUFFER + 1);
        printf("Awaiting a new connection\n");
        connSock = accept (listenSock, (struct sockaddr *) NULL, (socklen_t *) NULL);
        if (connSock == -1) {
            printf("accept() failed\n");
            perror("accept()");
            close(connSock);
            continue;
        } else {
            midLog("%s","New client connected....\n");
            string protocol_buffer = SeedInfoList.SerializeAsString();
            sprintf(buffer, "%s", protocol_buffer.c_str());
            datalen = strlen(buffer);
            ret = sctp_sendmsg(connSock, (void *) buffer, (size_t) datalen, NULL, 0, 0, 0, 0, 0, 0);
        }
        close(connSock);
    }
    return 0;
}