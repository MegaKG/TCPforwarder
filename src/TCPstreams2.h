//C++ TCPstreams Version 2.0

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string> 
#include <cstring>
#include <arpa/inet.h>
using namespace std;

//The Address Struct
struct TCPServer {
	struct sockaddr_in address;
	int fd;
};

struct TCPConnection {
	struct sockaddr_in address;
	int fd;
};


struct TCPServer* TCPopenserver(const char* IP, int port){
    int opt = 1;
    int server;
    
    struct TCPServer* MySock = (struct TCPServer*)malloc(sizeof(struct TCPServer));
    
     if (strcmp(IP,"0.0.0.0") == 0){
        MySock->address.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        inet_pton(AF_INET, IP, &MySock->address.sin_addr);
    }

    MySock->address.sin_family = AF_INET;
    MySock->address.sin_port = htons( port );


    

    //Open Socket
    server = socket(AF_INET, SOCK_STREAM, 0);
    
    //If Socket Open Failure
    if ( (server == 0) || (server == -1) ) {
    	free(MySock);
        perror("Socket Error 1:");
        return NULL; //Error 1
    }

    //Connect to Socket
    if ( setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))  ) {
    	free(MySock);
        perror("Socket Error 2:");
        return NULL; //Error 2
    }

    //bind to port
    if (bind(server, (struct sockaddr *)&MySock->address, sizeof(MySock->address)) < 0){
    	free(MySock);
        perror("Socket Error 3:");
        return NULL; //Error 3
    }

    //Now Listen on that port
    if (listen(server, 100) < 0){
    	free(MySock);
        perror("Socket Error 4:");
        return NULL; //Error 4
    }

    MySock->fd = server;
    return MySock;
}


struct TCPConnection* TCPaccept(struct TCPServer* server){
    int new_socket;
    struct TCPConnection* Con = (struct TCPConnection*)malloc(sizeof(struct TCPConnection));
    
    int sizeaddr = sizeof(Con->address);
    new_socket = accept(server->fd, (struct sockaddr *)&Con->address, (socklen_t*)&sizeaddr);
    if (new_socket < 0){
    	free(Con);
        perror("Socket Error 5:");
        return NULL; //Error 5
    }
    Con->fd = new_socket;
    return Con;
}

void TCPsenddat(struct TCPConnection* socketin, char* MSG){
    send(socketin->fd, MSG, strlen(MSG), 0);
}

char* TCPgetdat(struct TCPConnection* socketin, int buffer = 1024){
    char indat[buffer+1];
    memset(indat,0,buffer+1);

    //Now Read
    int result = read(socketin->fd, indat, buffer);

    if (result == 0){
        return NULL;
    }

    char* out = (char*)malloc(sizeof(char) * (result+1));
    strcpy(out,indat);

    return out;

}


struct TCPConnection* TCPopenclient(const char* IP, int port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct TCPConnection* Con = (struct TCPConnection*)malloc(sizeof(struct TCPConnection));

    if (sock < 0){
    	free(Con);
        perror("Socket Error 2:");
        return NULL; //Error 1
    }

    Con->address.sin_family = AF_INET;
    Con->address.sin_port = htons(port);

    //Convert IP Addresses
    if ( inet_pton(AF_INET, IP, &Con->address.sin_addr) <= 0 ){
        perror("Socket Error 2:");
        return NULL; // Error 2
    }
    //Connect to Socket
    if ( connect(sock, (struct sockaddr *)&Con->address, sizeof(Con->address) ) < 0  ){
        perror("Socket Error 3:");
        return NULL; //Error 3
    }

    Con->fd = sock;
    return Con;

}
