//C++ TCPstreams Version 2.0

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <cstring>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

using namespace std;

//The Address Struct
struct UNIXServer {
	struct sockaddr_un address;
	int fd;
};

struct UNIXConnection {
	struct sockaddr_un address;
	int fd;
};


struct UNIXServer* UNIXopenserver(char* Path){
    int opt = 1;
    int server;
    
    struct UNIXServer* MySock = (struct UNIXServer*)malloc(sizeof(struct UNIXServer));
    
    //Delete any old files
    remove(Path);

    //Clean the Struct
    memset(&MySock->address, 0, sizeof(struct sockaddr_un));
    
    //Copy our path in
    MySock->address.sun_family = AF_UNIX;
    strcpy(MySock->address.sun_path,Path);

    //Open Socket
    server = socket(AF_UNIX, SOCK_STREAM, 0);

    
    
    //If Socket Open Failure
    if ( (server == 0) || (server == -1) ) {
    	free(MySock);
        return NULL; //Error 1
    }

    //Connect to Socket
    if ( setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))  ) {
    	free(MySock);
        return NULL; //Error 2
    }

    //bind to port
    if (bind(server, (struct sockaddr *)&MySock->address, sizeof(MySock->address)) < 0){
    	free(MySock);
        return NULL; //Error 3
    }

    //Now Listen on that port
    if (listen(server, 100) < 0){
    	free(MySock);
        return NULL; //Error 4
    }

    MySock->fd = server;
    return MySock;
}


struct UNIXConnection* UNIXaccept(struct UNIXServer* server){
    int new_socket;
    struct UNIXConnection* Con = (struct UNIXConnection*)malloc(sizeof(struct UNIXConnection));
    
    int sizeaddr = sizeof(Con->address);
    new_socket = accept(server->fd, (struct sockaddr *)&Con->address, (socklen_t*)&sizeaddr);
    if (new_socket < 0){
    	free(Con);
        return NULL; //Error 5
    }
    Con->fd = new_socket;
    return Con;
}

void UNIXsenddat(struct UNIXConnection* socketin, char* MSG){
    //printf("FD %i\n",socketin->fd);
    send(socketin->fd, MSG, strlen(MSG), 0);
}

char* UNIXgetdat(struct UNIXConnection* socketin, int buffer = 1024){
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


struct UNIXConnection* UNIXopenclient(char* Path){
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct UNIXConnection* Con = (struct UNIXConnection*)malloc(sizeof(struct UNIXConnection));

    if (sock < 0){
    	free(Con);
        return NULL; //Error 1
    }

    //Clean the Struct
    memset(&Con->address, 0, sizeof(struct sockaddr_un));
    
    //Copy our path in
    Con->address.sun_family = AF_UNIX;
    strcpy(Con->address.sun_path,Path);

    //printf("Connecting to %s\n",Path);

    //Connect to Socket
    if ( connect(sock, (struct sockaddr *)&Con->address, sizeof(Con->address) ) < 0  ){
        return NULL; //Error 3
    }

    Con->fd = sock;
    return Con;

}
