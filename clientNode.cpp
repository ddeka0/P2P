#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#define MAX_BUFFER 3000
#define MY_PORT_NUM 54145 /* This can be changed to suit the need and should be same in server and client */

#include "logging.h"
#include "seedInfo.pb.h"

using namespace std;

int main (int argc, char* argv[]) {
    int connSock, in, i, ret, flags;
    struct sockaddr_in servaddr;
    struct sctp_status status;
    struct sctp_sndrcvinfo sndrcvinfo;
    char buffer[MAX_BUFFER + 1];
    int datalen = 0;

    connSock = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (connSock == -1) {
        printf("Socket creation failed\n");
        perror("socket()");
        exit(1);
    }
    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (MY_PORT_NUM);
    servaddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    
    if (ret == -1) {
        printf("Connection failed\n");
        perror("connect()");
        close(connSock);
        exit(1);
    }

    in = sctp_recvmsg (connSock, buffer, sizeof (buffer),(struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
    
    if(in == -1 ){
        higLog("%s"," sctp_recvmsg() failed");
    }else {
        SeedInfoServer::seedInfo SeedInfoList;
        buffer[in] = '\0';
        string protocol_buffer = buffer;
        SeedInfoList.ParseFromString(protocol_buffer);
        auto &Map = SeedInfoList.seedlist();
        for(auto e = Map.begin();e != Map.end();e++) {
            cout <<"hi" << endl;
        }
        printf("Successfully sent %d bytes data to server\n", in);
    }
    close (connSock);
    return 0;
}