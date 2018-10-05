#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <random>

#define MAX_BUFFER              3000
#define MY_PORT_NUM             60000
#define MAX_BACKLOG_REQUEST     100
#define MAX_NUM_PEER            4 
#include <netinet/in.h>
#include <openssl/sha.h>

/* This can be changed to suit the need and should be 
                            same in server and client */
#include "logging.h"
#include "seedInfo.pb.h"
#include "Message.pb.h"
//#include "sha1.h"
using namespace std;
// using google::protobuf;




bool running = false;
#define MSG_TYPE_GIVE_ME_PEER_LIST  1
#define MSG_TYPE_YOU_ARE_MY_PEER    2
#define MSG_TYPE_DATA               3
#define MSG_TYPE_PEER_LIST          4
#define WAIT_FOR_MY_SERVER_TO_START 10

#define FAILURE                     -1
#define SUCCESS                     0
string myIp = "10.129.135.201";/* FIXME : get my own IP address automatically */
map<string,string> seedListMap;
map<string,bool> ExistingMessage;   //can be modified
set<string> totalListofPeers;
set<string> listOfMyPeers;
vector<int> peerSocketsFds;         /*direct peers*/   
set<string> HashTable;

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
    servaddr.sin_addr.s_addr = inet_addr("10.129.135.192"); /* this is the IP of seedNodeListProvider*/
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
        // copy these IPs in the totalList of learned IPs
        for(auto &e:Map) {
            totalListofPeers.insert(e.second);
            higLog("%s","hi");
        }
        /* TODO how many entries received here ?? */
        /* FIXME : print nicely */
        higLog("Successfully got %d bytes data from seedInfoServer\n", in);
        higLog("Following is the seedNodeList");
        cout << SeedInfoList.DebugString() << endl;
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
    int in, i, ret, flags;
    struct sockaddr_in servaddr;
    struct sctp_status status;
    struct sctp_sndrcvinfo sndrcvinfo;
    char buffer[MAX_BUFFER + 1];
    int datalen = 0;

    connSock = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (connSock == -1) {
        higLog("%s"," Socket creation failed");
        return FAILURE;
    }
    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (MY_PORT_NUM);
    servaddr.sin_addr.s_addr = inet_addr(seedNodeServerIp.c_str());
    ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    /* connect may fail initially : TODO handle later */
    if (ret == -1) {
        higLog("%s"," Connection failed");
        close(connSock);
        return FAILURE;
    }else {
        lowLog("connection successfull to the seedNode %s",seedNodeServerIp.c_str());
    }
    return SUCCESS;
}

// void PeerTask(string peerAddr) {
//     LOG_ENTRY;
//     /* act as a client */
//     int connSock, in, i, ret, flags;
//     struct sockaddr_in servaddr;
//     struct sctp_status status;
//     struct sctp_sndrcvinfo sndrcvinfo;
//     char buffer[MAX_BUFFER + 1];
//     int datalen = 0;

//     connSock = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
//     if (connSock == -1) {
//         printf("Socket creation failed\n");
//         perror("socket()");
//         exit(1);
//     }
//     bzero ((void *) &servaddr, sizeof (servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons (MY_PORT_NUM);
//     servaddr.sin_addr.s_addr = inet_addr(peerAddr.c_str());
//     ret = connect (connSock, (struct sockaddr *) &servaddr, sizeof (servaddr));
    
//     if (ret == -1) {
//         printf("Connection failed\n");
//         perror("connect()");
//         close(connSock);
//         exit(1);
//     }else {
//         lowLog("%s"," connection successfull with the peer\n");
//     }
//     while(true) {
//         /*
//             1. generate one message
//             2. send to this peer
//             3. Store the message in the data structure
//         */
//     }
//     LOG_EXIT;
// }

int getpeerListFromSeedNodes() {
    LOG_ENTRY;
    /* iterate through all the seed nodes provided by seedInfoServer
       try to connect to them one by one
    */
    char buffer[MAX_BUFFER];
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
            sprintf(buffer, "%s", protocolBuffer.c_str());
            /* send this buffer over the sctp socket to the seedNode*/
            int ret = sendto(connSock, buffer, (size_t) datalen, 0,NULL,0);
            /* this send will invoke the processRequest() function and sendPeerList() */
            if(ret == -1) {
                higLog("%s"," sendto() failed");
                return FAILURE;
            }
            /* expecting a reply from the seedNode */
            memset(buffer,0,sizeof(buffer));
            in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
            /* received the totalListofPeers from the peer server/client :) */
            // Add this to the totalList
            if(in == -1 ) {
                cout << errno << endl;
                higLog("%s"," sctp_recvmsg() failed");
            }else {
                buffer[in] = '\0';
                string protocol_buffer = buffer;
                
                MP::BMessage msg1;
                msg1.ParseFromString(protocol_buffer);
                if(msg1.typeofmessage() != MSG_TYPE_PEER_LIST) {
                    higLog("Expected Peer List DATA: NOT found");
                    return FAILURE;
                }else {
                    higLog("received peer List from %s",e.second.c_str());
                    auto &Map = msg1.nodelist();
                    for(auto peer : Map) {
                        totalListofPeers.insert(peer.second);
                        /* if alreay exisits it wil be not inserted */
                    }
                }
            }
        }else {
            midLog("ignored Ip : %s",e.second.c_str());
        }
    }
    return SUCCESS;
}

void sendPeerList(int connSock) {
    LOG_ENTRY;
    char buffer[MAX_BUFFER];
    /* do not create thread */
    /* create a BMessage using the list : totalListofPeers in this NODE */
    MP::BMessage msg;
    msg.set_typeofmessage(MSG_TYPE_PEER_LIST);
    auto& Map = *msg.mutable_nodelist();
    higLog("Total length of the list is = %d",totalListofPeers.size());
    for(string peer : totalListofPeers) {
        Map[peer] = peer;   /* key is not important here */
    }
    string protocolBuffer = msg.SerializeAsString();
    int datalen = protocolBuffer.length();
    sprintf(buffer, "%s", protocolBuffer.c_str());
    int ret = sendto(connSock, buffer, (size_t) datalen, 0,NULL,0);
    if(ret == -1) {
        higLog("%s"," sendto() failed aborting !!!");
        return;
    }else {
        higLog("Following list is sent to the client :");
        cout << msg.DebugString() << endl;
    }
    sleep(1);
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
            continue;
        }
        buffer[in] == '\0';
        string protocolBuffer = buffer;
        MP::BMessage msg;
        msg.ParseFromString(protocolBuffer);

        midLog("received %s",msg.msg().c_str());
        continue;

        if(msg.typeofmessage() != MSG_TYPE_DATA) {
            higLog("%s"," Not supposed to recv other then actual data");
        }else {
            midLog("received %s",msg.msg().c_str());
            unsigned char hash[SHA_DIGEST_LENGTH];
            SHA1(reinterpret_cast<const unsigned char *>(msg.msg().c_str()), msg.msg().length(), hash);
            std::string str(hash,hash + SHA_DIGEST_LENGTH);
            int datalen = msg.msg().length();
            if(HashTable.find(str) == HashTable.end()) {
                HashTable.insert(str);
                /* send this message to all 4 peers */
                for(int fd : peerSocketsFds) {
                    int ret = sendto(connSock, buffer, (size_t) datalen, 0,NULL,0);
                    if(ret == -1) {
                        higLog("%s"," sendto() failed");
                        return;
                    } 
                    higLog("%s",to_string(ret) + " bytes send to one peer");
                }
            }else{
                /* Don't send this message to any peers */
            }
        }
    }
    LOG_EXIT;    
}

int processRequest(string requestBuffer,int connSock) {
    LOG_ENTRY;
    MP::BMessage msg;
    msg.ParseFromString(requestBuffer);
    int type = msg.typeofmessage();
    if(type == MSG_TYPE_GIVE_ME_PEER_LIST) {
        midLog("found request of type MSG_TYPE_GIVE_ME_PEER_LIST");
        sendPeerList(connSock);
        close(connSock);
    }else if(type == MSG_TYPE_YOU_ARE_MY_PEER) {
        midLog("found request of type MSG_TYPE_YOU_ARE_MY_PEER");
        /* follwoing function will loop infinitely */
        acceptPeerRequstAndProcess(connSock);
    }
    LOG_EXIT;
    return SUCCESS;
}

void executeOwnWork() {
    LOG_ENTRY;
    auto rng = std::default_random_engine {};

    higLog("Asking the seedInfoSever for the seedNodeList ...... ");
    if(getSeedNodeList() < 0) {
        higLog("%s"," getSeedNodeList failed");
        //exit(1);
        std::terminate();
    }
    
    sleep(WAIT_FOR_MY_SERVER_TO_START);
    higLog("%s"," executeOwnWork() started");
    /*
        ANY PEER : seedNode or normal client must ask for seedNodeList from 
        seedInfoServer
       <=========ASK the seedInfoServer for the List of seedNodes==========>
    */
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
        higLog("%s"," getpeerListFromSeedNodes() failed");
        //exit(1);
        std::terminate();
    }
    /* Got a final list of possible peers
       Now we need to select 4 peers randomly
    */
    /* shuffel the vector totalListofPeers */ 
    
    vector<string> tempList(totalListofPeers.begin(),totalListofPeers.end());
    std::shuffle(std::begin(tempList), std::end(tempList), rng);
    
   for(int i = 0;i < min((int)tempList.size() , MAX_NUM_PEER);i++) {
        // listOfMyPeers.push_back(tempList[i]);
        listOfMyPeers.insert(tempList[i]); /* listOfMyPeers is a set */
    }
    // prepapre the socket fd for final connextions 
    for(string peerIp : listOfMyPeers) {
        if(peerIp != myIp) {
            int connSock;
            connectToASeedNode(connSock,peerIp);    // Name needed to be changed
            peerSocketsFds.push_back(connSock);
        }
    }
    higLog("size of peerSocketsFds = %d",peerSocketsFds.size());
    char buffer[MAX_BUFFER];
    long long int cnt = 0;
    while(true) {
        /* generate a random message */
        MP::BMessage msg;
        msg.set_typeofmessage(MSG_TYPE_DATA);
        /* generate a random message */
        string dataMsg = "Hello " + to_string(cnt);
        msg.set_msg(dataMsg);
        string protocolBuffer = msg.SerializeAsString();
        int datalen = protocolBuffer.length();
        sprintf(buffer, "%s", protocolBuffer.c_str());
        for(int peerFD : peerSocketsFds) {
            int ret = sendto(peerFD, buffer, (size_t) datalen, 0,NULL,0);
            if(ret == -1) {
                higLog("sendto() failed");
                continue;
            }else {
                higLog("sent to %d",peerFD);
            }
        }
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char *>(msg.msg().c_str()), msg.msg().length(), hash);
        std::string str(hash,hash + SHA_DIGEST_LENGTH);
        HashTable.insert(str);
        cnt++;
        memset(buffer,0,sizeof(buffer));
    }
    LOG_EXIT;
}

int main (int argc, char* argv[]) {
    LOG_ENTRY;
    std::thread myTask(executeOwnWork);/*detach or join : decide and fix later*/
    myTask.detach();
    /*
        First create a server socket that listens in a while loop
        recv packets -- process -- executes
       <=======================================================================>
            start the server
       <=======================================================================>    
    
    */
    int listenSock, connSock, ret, in, flags, i;
    struct sockaddr_in servaddr,client;
    struct sctp_initmsg initmsg;
    struct sctp_event_subscribe events;
    struct sctp_sndrcvinfo sndrcvinfo;
    char buffer[MAX_BUFFER + 1];
    int datalen = 0;
    string clientIp;
    int client_address_len = sizeof(struct sockaddr_in);

    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if(listenSock == -1) {
        higLog("%s","Failed to create socket");
        exit(1);
    }
    int opt = SO_REUSEADDR;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        higLog("%s","setsockopt() failed");
        exit(EXIT_FAILURE);
    }else {
        higLog("%s"," setsockopt success");
    }

    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(myIp.c_str());
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
    running = true;
    while(running) {
        char buffer[MAX_BUFFER + 1];
        int len;
        bzero (buffer, MAX_BUFFER + 1);
        midLog("%s","Awaiting a new connection ...........");
        connSock = accept (listenSock, (struct sockaddr *)&client, (socklen_t *)&client_address_len);
        /* FIXME remove NULL and find out the ip of the clients and print */
        if (connSock == -1) {
            printf("accept() failed\n");
            perror("accept()");
            close(connSock);
            continue;
        }else {
            clientIp = inet_ntoa(client.sin_addr);
            higLog("Request came from client : %s",clientIp.c_str());
        }
        in = recvfrom(connSock, buffer, sizeof (buffer), 0, NULL, NULL);
        if(in == -1 ){
            cout << errno << endl;
            higLog("%s"," sctp_recvmsg() failed");
        }else {
            midLog("request recved from client :%s",clientIp.c_str());
            buffer[in] = '\0';
            string requestBuffer = buffer;
            std::thread newRequestThread(processRequest,requestBuffer,connSock);
            newRequestThread.detach();
        }
    }   /* server is running now */

    LOG_EXIT;
    return 0;
}