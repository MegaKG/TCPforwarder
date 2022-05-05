#include <iostream>
#include "TCPstreams.hpp"
#include "StringInputs2.h"
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
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
    struct args* In = (struct args*)VIn;
    int serv = In->from;
    int clie = In->to;

    char Buffer[bufsize];
    int haveread;

    while (In->MyCon->Status){
    //while (1){
        haveread = read(serv, Buffer, bufsize);
        //printf("%i\n",haveread);
        //printf("RECV\n");
        if (haveread <= 0){
            break;
        }
        //else if ((haveread != 0) && (Buffer[0] == 0)){
        //    break;
        //}
        else {
            send(clie, Buffer, haveread, 0);
        }
        
    }
    In->MyCon->Status = 0;
}

void* clienthandle(void* VIn){
    struct connection* MyCon = (struct connection*)VIn;
    int myid = MyCon->ID;
    printf("%i -> Thread Start\n",myid);
    
    //Open To Server
    MyCon->ClientCon = openclient(DestIP,DestPort);
    if (MyCon->ClientCon > 0){
        printf("%i -> Success Opening Client\n",myid);

        struct args* C2SA = (struct args*)malloc(sizeof(struct args));
        struct args* S2CA = (struct args*)malloc(sizeof(struct args));
        C2SA->from = MyCon->ClientCon;
        C2SA->to = MyCon->ServerCon;
        C2SA->MyCon = MyCon;

        S2CA->to = MyCon->ClientCon;
        S2CA->from = MyCon->ServerCon;
        S2CA->MyCon = MyCon;
        printf("%i -> Gen Connection Data\n",myid);

        pthread_t* C2S = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(C2S, NULL, &forwarder, C2SA);
        printf("%i -> Start C2S\n",myid);

        pthread_t* S2C = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(S2C, NULL, &forwarder, S2CA);
        printf("%i -> Start S2C\n",myid);
        
        MyCon->C2ST = C2S;
		MyCon->S2CT = S2C;
		MyCon->C2SA = C2SA;
		MyCon->S2CA = S2CA;

        while (MyCon->Status){
            sleep(1);
            //printf("-> Await\n");
        }
        printf("%i -> Status Changed\n",myid);
        

        //forwarder(S2CA);
        printf("%i -> Reached Termination\n",myid);
    }
    else {
        printf("%i -> Connection Failure\n",myid);
    }

    
    printf("%i -> Died\n",myid);
    
}

//The Grim Reaper
void garbageCollector(){
    vector<int> ToDel;

    for (int i = 0; i < Cons.size(); i++){
        if (Cons[i]->Status == 0){
            ToDel.push_back(i);
        }
    }

	void* val;
    for (int i = ToDel.size() - 1; i > -1 ; i--){
        //Cleanup Code Here
        printf("Kill Client Thread %i\n",ToDel[i]);
        
        //The Children of this Thread First
        
        //Kill the Connections
        close(Cons[ToDel[i]]->ClientCon);
        close(Cons[ToDel[i]]->ServerCon);
        printf("Closed Connections %i\n",ToDel[i]);
        
        //Now Terminate the Threads
        pthread_cancel(*Cons[ToDel[i]]->C2ST);
        pthread_cancel(*Cons[ToDel[i]]->S2CT);
        printf("Cancelled Child Threads %i\n",ToDel[i]);
        
        //Await the result
        pthread_join(*Cons[ToDel[i]]->C2ST,&val);
        pthread_join(*Cons[ToDel[i]]->S2CT,&val);
        printf("Joined Child Threads %i\n",ToDel[i]);

        //Clean Up the Dangling Pointers
        free(Cons[ToDel[i]]->C2SA);
        free(Cons[ToDel[i]]->S2CA);
        free(Cons[ToDel[i]]->C2ST);
        free(Cons[ToDel[i]]->S2CT);
        printf("Freed Child Pointers %i\n",ToDel[i]);
        
        
        //The Main Thread Stuff
        pthread_cancel(*Threads[ToDel[i]]);
        printf("Cancelled %i\n",ToDel[i]);
        pthread_join(*Threads[ToDel[i]],&val);
        printf("Joined %i\n",ToDel[i]);

        free(Cons[ToDel[i]]);
        free(Threads[ToDel[i]]);

        Threads.erase(Threads.begin() + ToDel[i]);
        Cons.erase(Cons.begin() + ToDel[i]);
        
        

    }

}

int main(int argc, char** argv){
    int cserver;
    int ccon;
    int counter = 0;

    if (argc < 6){
        printf("Usage:\n%s <IP HOST> <PORT HOST> <IP TARGET> <PORT TARGET> <BUFFER SIZE>\nEG: %s 0.0.0.0 5000 192.168.1.4 3000 1024\n", argv[0], argv[0]);
        printf("This would host on 0.0.0.0:5000 and forward to 192.168.1.4:3000 with buffer of 1024\n");
        return -1; 
    }
    
    HostIP.assign(argv[1]);
    DestIP.assign(argv[3]);
    HostPort = stosi(argv[2]);
    DestPort = stosi(argv[4]);
    bufsize = stosi(argv[5]);

    printf("Forwarding %s:%i to %s:%i buffer %i\n",HostIP.c_str(),HostPort,DestIP.c_str(),DestPort,bufsize);
    cserver = openserver(HostIP,HostPort);
    while (true){
        usleep(1000);
        printf("Running Objects: %i\n",Threads.size());
        printf("Awaiting Connection\n");
        ccon = accept(cserver);
        if (ccon <= 0){
            printf("Accept Failure\n");
            sleep(1);
        }
        else {
            printf("Accepted Client %i\n",counter);
            

            //Create Connection info
            struct connection* NewCon = (struct connection*)malloc(sizeof(struct connection));
            NewCon->Server = cserver;
            NewCon->ServerCon = ccon;
            NewCon->Status = 1;
            NewCon->ClientCon = 0;
            NewCon->ID = counter;
            Cons.push_back(NewCon);
            printf("Made Connection\n");
            counter += 1;

            //Create the thread
            pthread_t* NewTh = (pthread_t*)malloc(sizeof(pthread_t));
            pthread_create(NewTh, NULL, &clienthandle, NewCon);
            
            printf("Thread CR\n");
            Threads.push_back(NewTh);
            printf("Started Thread\n");

            garbageCollector();
        }

    }
}
