#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
    servaddr.sin_addr.s_addr = inet_addr("10.129.135.192");
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    
    if (ret == -1) {
        printf("Connection failed\n");
        perror("connect()");
        close(connSock);
        exit(1);
    }else {
        lowLog("%s"," connection successfull\n");
    }
/*     bool received = false;
    while(!received) {
        in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
        if(in != -1) {

        }
    } */
    in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
    if(in == -1 ){
        cout << errno << endl;
        higLog("%s"," sctp_recvmsg() failed");
    }else {
        SeedInfoServer::seedInfo SeedInfoList;
        buffer[in] = '\0';
        string protocol_buffer = buffer;
        SeedInfoList.ParseFromString(protocol_buffer);
        auto &Map = SeedInfoList.seedlist();
        for(auto & e: Map) {
            cout << e.first << endl;
            cout << e.second << endl;
        }
        printf("Successfully got %d bytes data from server\n", in);
    }
    close (connSock);
    return 0;
}