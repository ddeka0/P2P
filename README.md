CN-HW2

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
    Create 4 containers for 4 seedNodes. 
    Create 1 container for 1 seedInfoServer
    Create Any number of containers for client testing
    Configure the IPs manually
    
    run seedInfoServer by running the command ./seedInfoServer
    run clientNode     by running the command ./clientNode
        
Notes : 
     
    
    
    
