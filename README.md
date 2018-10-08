CN-HW2
demo link : https://youtu.be/mPIF7muzTGY

Requirements : 

    System Requirement:
        - Linux Kernel 2.6 or higher
        - g++ 6+ or later {c++14}
    
    Dependency Package:
        - google protocol buffer 3.6
        - libsctp-dev 
        - libssl-dev
        - make
        - lxd
        

How To Build ?

    - cd CN-HW2
    
    Run the following commands as root:
    
    - to build seedInfoServer
        - g++ -std=c++14 seedInfoServer.cpp seedInfo.pb.cc logging.cpp -o seedInfoServer -lsctp `pkg-config --cflags --libs protobuf` -DCOLOURED_LOGS
        
    - to build clientNode
        - g++ -std=c++14 clientNode.cpp seedInfo.pb.cc Message.pb.cc logging.cpp -o clientNode -lsctp -lssl -lcrypto `pkg-config --cflags --libs protobuf` -DCOLOURED_LOGS
    
How To Run Binary ?
    
    - Create 4 containers for 4 seedNodes. 
    - Create 1 container for 1 seedInfoServer
    - Create Any number of containers for client testing
    - Configure the IPs manually
    
    run seedInfoServer by running the command ./seedInfoServer
    run clientNode     by running the command ./clientNode
        
Notes :
I used a common message format across all types of message transfer:


    message {
        int32 typeofmessage.
        string msg.
        map<string,string> ListofNodes.
    }

typeofmessage can take 4 values.

    - 1. Give me peer list : MSG_TYPE_GIVE_ME_PEER_LIST
    - 2. You are my peer   : MSG_TYPE_YOU_ARE_MY_PEER
    - 3. Actual data (bitcoin message) : MSG_TYPE_DATA
    - 4. peer list : MSG_TYPE_PEER_LIST

    Message encoding and decoding is done by Google Protocol Buffer
    SCTP was used. it helps in the message oriented packer transfer instead of byte oriented transfer

Each client has the following three types of threads.Therefore each node has at least three threads.
    
    - main thread that listen and accept connection from a new client.Types of requests are :
        - Give peers list
        - make peerNode
    - Handler Threads ( the number of this type of thread is unbounded). It recv bitcoin messages coming from some node and replayes to it's own peers
    - PeerNode Thread. It generates new messages at an interval of 5 seconds and broadcasts to all the 4 peers.
    
 
    
    
    
