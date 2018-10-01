#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#define MAX_BUFFER              3000
#define MY_PORT_NUM             54145
#define MAX_BACKLOG_REQUEST     100 
#include <netinet/in.h>
#include <openssl/sha.h>

/* This can be changed to suit the need and should be 
                            same in server and client */
#include "logging.h"
#include "seedInfo.pb.h"
#include "Message.pb.h"
#include "sha1.h"
using namespace std;
// using google::protobuf;




bool running = false;
#define MSG_TYPE_GIVE_ME_PEER_LIST  1
#define MSG_TYPE_YOU_ARE_MY_PEER    2
#define MSG_TYPE_DATA               3
#define WAIT_FOR_MY_SERVER_TO_START 2

#define FAILURE                     -1
#define SUCCESS                     0
string myIp = "10.129.135.201";/* FIXME : get my own IP address automatically */
map<string,string> seedListMap;
map<string,bool> ExistingMessage;   //can be modified
set<string> listOfMyPeers;
set<int> peerSocketsFds;   // type1 peer (my peers)

void receiveAndSend() {
    // this is also an infinite loop function
    // this thread will loop in a while loop
    // recv() for incoming message 
    // process that message
    // check the Map data structure
    // if not found then send to all its peers
    // repeat this process infinitely
    while(running) {
        
    }  
}


int getSeedNodeList() {
    LOG_ENTRY;
    int connSock, in, i, ret, flags;
    struct sockaddr_in servaddr;
    struct sctp_status status;
    struct sctp_sndrcvinfo sndrcvinfo;
    char buffer[MAX_BUFFER + 1];
    int datalen = 0;
    connSock = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (connSock == -1) {
        /* FIXME : print nicely */
        printf("Socket creation failed\n");
        perror("socket()");
        return FAILURE;
    }
    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (MY_PORT_NUM);
    servaddr.sin_addr.s_addr = inet_addr("10.129.135.192");
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    if (ret == -1) {
        /* FIXME : print nicely */
        printf("Connection failed\n");
        perror("connect()");
        close(connSock);
        return FAILURE;
    }else {
        lowLog("%s"," connection successfull\n");
    }
    /* FIXME try multiple times : untill one read sucess
       ( single try may fail with error code 11 resource not available )
       Because we need only one recv success from the seedInfoServer
       We are using sctp : therefore we willl recv the entire message in one go
    */
    in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL); 
    /*  FIXME 
        use proper functions
        get the IP from the sender and print it to the console
    */
    if(in == -1) {
        /* FIXME : print nicely */
        cout << errno << endl;
        higLog("%s"," sctp_recvmsg() failed");
        /* FIXME do we need to return some error value form this place ? */
    }else {
        SeedInfoServer::seedInfo SeedInfoList;
        buffer[in] = '\0';
        string protocol_buffer = buffer;
        SeedInfoList.ParseFromString(protocol_buffer);
        auto &Map = SeedInfoList.seedlist();
        /* FIXME is it required to take a lock 
            before accessing the seedListMap here
        */
        for(auto & e: Map) {
            seedListMap[e.first] =  e.second;
        }
        /* TODO how many entries received here ?? */
        /* FIXME : print nicely */
        printf("Successfully got %d bytes data from server\n", in);
    }
    close (connSock);

    LOG_EXIT;
    return SUCCESS;
}

int connectToASeedNode(int &connSock,string seedNodeServerIp) {
    /*
        1. Create socket : store the socketFD in the connSock arg
        2. Connect to the serverIp (it is a seedNode server)
    */
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
        return FAILURE;
    }
    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (MY_PORT_NUM);
    servaddr.sin_addr.s_addr = inet_addr(seedNodeServerIp.c_str());
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    
    if (ret == -1) {
        printf("Connection failed\n");
        perror("connect()");
        close(connSock);
        return FAILURE;
    }else {
        lowLog("%s"," connection successfull to the seedNode" + serverIp);
    }
    return SUCCESS;
}

void PeerTask(string peerAddr) {
    LOG_ENTRY;
    /* act as a client */
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
    servaddr.sin_addr.s_addr = inet_addr(peerAddr.c_str());
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    
    if (ret == -1) {
        printf("Connection failed\n");
        perror("connect()");
        close(connSock);
        exit(1);
    }else {
        lowLog("%s"," connection successfull with the peer\n");
    }
    while(true) {
        /*
            1. generate one message
            2. send to this peer
            3. Store the message in the data structure
        */
    }
    LOG_EXIT;
}

int getpeerListFromSeedNodes() {
    LOG_ENTRY;
    /* iterate through all the seed nodes provided by seedInfoServer
       try to connect to them one by one
    */
    char bufer[MAX_BUFFER];
    int in = 0;
    for(auto &e:seedListMap) {
        int connSock;
        size_t datalen = 0;
        if(e.second != myIp) {
            if(connectToASeedNode(connSock,e.second) < 0 ) {
                higLog("%s"," connect to " + e.second + " failed");
                /* FIXME later */
            }
            /* preprare a message : give peer list */
            MP::BMessage msg;
            msg.set_typeofmessage(MSG_TYPE_GIVE_ME_PEER_LIST);
            msg.set_msg("");
            string protocolBuffer = msg.SerializeAsString();
            datalen = protocolBuffer.length();
            sprintf(buffer, "%s", protocol_buffer.c_str());
            /* send this buffer over the sctp socket to the seedNode*/
            ret = sendto(connSock, buffer, (size_t) datalen, 0,NULL,0);

            /* expecting a reply from the seedNode */
            memset(buffer,0,sizeof(buffer));
            in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
            if(in == -1 ) {
                cout << errno << endl;
                higLog("%s"," sctp_recvmsg() failed");
            }
            // Parse this message and get the list of peers
            // Add this to the totalList
        }
    }
    return SUCCESS;
}
void sendPeerList() {
    LOG_ENTRY;
    /* do not create thread */



    LOG_EXIT;
}
void acceptPeerRequstAndProcess(int connSock) {
    LOG_ENTRY;
    /* create a thread to receive the messages coming from this peer */
    int in = 0;
    char buffer[MAX_BUFFER];
    memset(buffer,0,sizeof(buffer));
    while(true) {
        in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
        if(in == -1) {
            higLog("%s"," error in recvfrom()");
        }
        buffer[in] == '\0';
        string protocolBuffer = buffer;
        MP::BMessage msg;
        msg.ParseFromString(protocolBuffer);
        if(msg.typeofmessage() != MSG_TYPE_DATA) {
            higLog("%s"," Not supposed to recv other then actual data");
        }else {
            /*
                1. recved a actual data string
                2. find hash(string)
                3. check the existing MAP
                4. If not found
                    4.1 Store it in the MAP
                    4.2 Send to all the four peers
                    4.3 write to file also
                    Dont create a thread
                    do all here only 
            */
           const unsigned char str[] = msg.msg();
           unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
           SHA1(str, sizeof(str) - 1, hash);
           if(!ExistingMessage[hash]){
               ExistingMessage[hash] = true;
               //send this message to all 4 peers
                for(int fd : peerSocketsFds) {
                    int ret = sendto(connSock, buffer, (size_t) datalen, 0,NULL,0);
                    // handle error

                }
           }else{
               //Don't send this message to any peers
           }




        }
    }
    LOG_EXIT;    
}

int processRequest(string requestBuffer,int connSock) {
    LOG_ENTRY;
    MP::BMessage msg;
    msg.ParseFromString(requestBuffer);
    int type = msg.typeOfMessage();
    if(type == MSG_TYPE_GIVE_ME_PEER_LIST) {
        sendPeerList(connSock);
        close(connSock);
    }else if(type == MSG_TYPE_YOU_ARE_MY_PEER) {
        /* maintain the connSock
           create a separate thread to handle this peer of type 2
           I need to loop in a while loop
           recv() messages from the peer
           check in the data structure
           if not found save in the data structure
           if found : forget this message
           repreat to recv()     
        */
       
        std::thread handleReplayMessage(acceptPeerRequstAndProcess,connSock);
    }
    LOG_EXIT;
    return SUCCESS;
}

void executeOwnWork() {

    sleep(WAIT_FOR_MY_SERVER_TO_START);
    /*
        ANY PEER : seedNode or normal client must ask for seedNodeList from 
        seedInfoServer
       <=========ASK the seedInfoServer for the List of seedNodes==========>
    */
    if(getSeedNodeList() < 0) {
        higLog("%s"," getSeedNodeList failed");
        exit(1);
    }
    /* Got the seedNodeList from the seedInfoServer and
       stored in a data structure or list
    */

    /*
       <=======================================================================>
            Connect to the seed Nodes : take IPs from the stored list one by one
            Ask seedNode about the list of peers
       <=======================================================================>
    */
    if(getpeerListFromSeedNodes() < 0) {
        higLog("%s"," establishConnectionWithPeers failed");
        exit(1);
    }
    /* Got a final list of possible peers
       Now we need to select 4 peers randomly
    */

    // final lise of peers [ includes seedNode as well as normal client ]
    // Select any 4 of them randomly
    // Send "you are my peer" message to the 4 selected peers
    // start separate thread (4) for all of them
    // those threads will do the following:
    /*
        1. Create a TCP connection with the peerAddr
        2. Loop in a infite loop
        3. Generate BITCOIN transaction Message at an interval of 5 sec.
        4. send these message to the peerAddr
    */
    for(int peer = 1;peer <= 4;peer++) {
        /* will it better to create just one thread : and that will broadcast the
         messages to all 4 peers
        */
        std::thread handlePeerThread(PeerTask,peerAddr);
    }
    LOG_EXIT;
}

int main (int argc, char* argv[]) {
    LOG_ENTRY;
    std::thread myTask(executeOwnWork); /* detach or join : decide and fix later */
    
    /*
        First create a server socket that listens in a while loop
        recv packets -- process -- executes
       <=======================================================================>
            start the server
       <=======================================================================>    
    
    */
   

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
    servaddr.sin_addr.s_addr = inet_addr("10.129.135.192");
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

    while(running) {
        char buffer[MAX_BUFFER + 1];
        int len;
        bzero (buffer, MAX_BUFFER + 1);
        printf("Awaiting a new connection\n");
        connSock = accept (listenSock, (struct sockaddr *) NULL, (socklen_t *) NULL);
        /* FIXME remove NULL and find out the ip of the clients and print */
        if (connSock == -1) {
            printf("accept() failed\n");
            perror("accept()");
            close(connSock);
            continue;
        }
        in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
        if(in == -1 ){
            cout << errno << endl;
            higLog("%s"," sctp_recvmsg() failed");
        }else {
            buffer[in] = '\0';
            string requestBuffer = buffer;
            std::thread newRequestThread(processRequest,requestBuffer,connSock);
        }
        /* close(connSock);  No need to close the connection now 
                            processRequest() will do the required stuff
                         */
        /*  close this connection and go on accepting other requests */
    }   /* server is running now */


    LOG_EXIT;
    return 0;
}