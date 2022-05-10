#include <iostream>
#include "TCPstreams2.hpp"
#include "StringInputs2.h"
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
using namespace std;

//Filler Initial Values, to be initialised
string HostIP = "127.0.0.1";
string DestIP = "127.0.0.1";
int HostPort = 5000;
int DestPort = 5000;
int bufsize = 1024;


struct args {
  int to;
  int from;
  struct connection* MyCon;
};

struct connection {
  int Server;
  int ServerCon;
  int ClientCon;
  int Status;  
  int ID;
  
  //These Exist only to be cleared
  pthread_t* C2ST;
  pthread_t* S2CT;
  struct args* C2SA;
  struct args* S2CA;
};

vector <connection*> Cons;
vector <pthread_t*> Threads;

void* forwarder(void* VIn){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can detect and fix these properly
    struct args* In = (struct args*)VIn;
    int serv = In->from;
    int clie = In->to;

    char Buffer[bufsize];
    int haveread;

    while (In->MyCon->Status){
        haveread = read(serv, Buffer, bufsize);
        if (haveread <= 0){
            break;
        }
        else {
            send(clie, Buffer, haveread, 0);
        }
        
    }
    In->MyCon->Status = 0;
}

void* clienthandle(void* VIn){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can detect and fix these properly
    struct connection* MyCon = (struct connection*)VIn;
    int myid = MyCon->ID;
    
    //Open To Server
    struct TCPConnection* NewCon = openclient(DestIP,DestPort);
    //Extract FD
    MyCon->ClientCon = NewCon->fd;
    //Here we deallocate the connection struct pointer as we have already extracted the file descriptor
    free(NewCon);

    //Check for Bad Connection
    if (MyCon->ClientCon != NULL){
        printf("%i -> Success Opening Client\n",myid);

        //Here we Set up the arguments for the client to server and server to client threads
        struct args* C2SA = (struct args*)malloc(sizeof(struct args));
        struct args* S2CA = (struct args*)malloc(sizeof(struct args));
        C2SA->from = MyCon->ClientCon;
        C2SA->to = MyCon->ServerCon;
        C2SA->MyCon = MyCon;

        S2CA->to = MyCon->ClientCon;
        S2CA->from = MyCon->ServerCon;
        S2CA->MyCon = MyCon;

        //Now start the threads
        pthread_t* C2S = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(C2S, NULL, &forwarder, C2SA);

        pthread_t* S2C = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(S2C, NULL, &forwarder, S2CA);
        
        //We keep a record in the global scope so the garbageCollector can destroy / clean them up
        MyCon->C2ST = C2S;
		MyCon->S2CT = S2C;
		MyCon->C2SA = C2SA;
		MyCon->S2CA = S2CA;

        while (MyCon->Status){
            sleep(1);
        }
        
        printf("%i -> Reached Termination\n",myid);
    }
    else {
        printf("%i -> Connection Failure\n",myid);
    }

    
    printf("%i -> Died\n",myid);
    
}

//The Grim Reaper
void garbageCollector(){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can detect and fix these properly
    //Dead Thread / Connection Indexes are stored in this vector
    vector<int> ToDel;

    for (int i = 0; i < Cons.size(); i++){
        if (Cons[i]->Status == 0){
            ToDel.push_back(i);
        }
    }

    //After finding the dead threads / connections, we kill them off
	void* val;
    for (int i = ToDel.size() - 1; i > -1 ; i--){
        //Cleanup Code Here
        
        //The Children of this Thread First
        
        //Kill the Connections
        close(Cons[ToDel[i]]->ClientCon);
        close(Cons[ToDel[i]]->ServerCon);

        
        //Now Terminate the Threads
        pthread_cancel(*Cons[ToDel[i]]->C2ST);
        pthread_cancel(*Cons[ToDel[i]]->S2CT);
        
        //Await the result
        pthread_join(*Cons[ToDel[i]]->C2ST,&val);
        pthread_join(*Cons[ToDel[i]]->S2CT,&val);

        //Clean Up the Dangling Pointers
        free(Cons[ToDel[i]]->C2SA);
        free(Cons[ToDel[i]]->S2CA);
        free(Cons[ToDel[i]]->C2ST);
        free(Cons[ToDel[i]]->S2CT); 
        
        //The Main Thread Stuff
        pthread_cancel(*Threads[ToDel[i]]);
        pthread_join(*Threads[ToDel[i]],&val);

        //Clean up Dangling Pointers
        free(Cons[ToDel[i]]);
        free(Threads[ToDel[i]]);

        //Finally Destroy their references in the Connection and Thread Arrays
        Threads.erase(Threads.begin() + ToDel[i]);
        Cons.erase(Cons.begin() + ToDel[i]);
        
    }

}

int main(int argc, char** argv){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can fix these properly
    struct ServerSocketData* cserver;
    struct TCPConnection* ccon;
    int counter = 0;

    if (argc < 6){
        printf("Usage:\n%s <IP HOST> <PORT HOST> <IP TARGET> <PORT TARGET> <BUFFER SIZE>\nEG: %s 0.0.0.0 5000 192.168.1.4 3000 1024\n", argv[0], argv[0]);
        printf("This would host on 0.0.0.0:5000 and forward to 192.168.1.4:3000 with buffer of 1024\n");
        return -1; 
    }
    
    //Loads CMDline Arguments
    HostIP.assign(argv[1]);
    DestIP.assign(argv[3]);
    HostPort = stosi(argv[2]);
    DestPort = stosi(argv[4]);
    bufsize = stosi(argv[5]);

    printf("Forwarding %s:%i to %s:%i buffer %i\n",HostIP.c_str(),HostPort,DestIP.c_str(),DestPort,bufsize);
    cserver = openserver(HostIP,HostPort);
    while (true){
        //usleep(1000);
        printf("Running Objects: %i\n",Threads.size());
        printf("Awaiting Connection\n");
        ccon = accept(cserver);
        if (ccon == NULL){
            printf("Accept Failure\n");
            sleep(1);
        }
        else {
            //Accept a Client
            printf("IP Connected %s id %i\n",inet_ntoa(ccon->address.sin_addr),counter);
            

            //Create Connection arguments to be passed to the handler
            struct connection* NewCon = (struct connection*)malloc(sizeof(struct connection));
            NewCon->Server = cserver->fd;
            NewCon->ServerCon = ccon->fd;
            NewCon->Status = 1;
            NewCon->ClientCon = 0;
            NewCon->ID = counter;
            Cons.push_back(NewCon);

            //Create the thread
            pthread_t* NewTh = (pthread_t*)malloc(sizeof(pthread_t));
            pthread_create(NewTh, NULL, &clienthandle, NewCon);
            
            //Keep a Record of the new thread
            Threads.push_back(NewTh);
            printf("Started Thread for %i\n",counter);
            
            //Clean up the dangling pointer of the initial connection struct
            free(ccon);

            garbageCollector();
            counter += 1;
        }

    }
}
