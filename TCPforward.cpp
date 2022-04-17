#include <iostream>
#include "TCPstreams.hpp"
#include "StringInputs.h"
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
using namespace std;

const string HostIP = "0.0.0.0";
const string DestIP = "127.0.0.1";
const int HostPort = 5000;
const int DestPort = 5001;
const int bufsize = 1024;

void loadConfig(){

}

struct connection {
  int Server;
  int ServerCon;
  int ClientCon;
  int Status;  
  int ID;
};

struct args {
  int to;
  int from;
  struct connection* MyCon;
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
        //printf("RECV\n");
        if (haveread == 0){
            break;
        }
        else {
            send(clie, Buffer, haveread, 0);
        }
        
    }
    In->MyCon->Status = 0;
}

void* clienthandle(void* VIn){
    //printf("Thread Start\n");
    struct connection* MyCon = (struct connection*)VIn;
    //Open To Server
    MyCon->ClientCon = openclient(DestIP,DestPort);
    //printf("Success Opening Client %i\n",MyCon->ID);

    struct args* C2SA = (struct args*)malloc(sizeof(struct args));
    struct args* S2CA = (struct args*)malloc(sizeof(struct args));
    C2SA->from = MyCon->ClientCon;
    C2SA->to = MyCon->ServerCon;
    C2SA->MyCon = MyCon;

    S2CA->to = MyCon->ClientCon;
    S2CA->from = MyCon->ServerCon;
    S2CA->MyCon = MyCon;
    //printf("Gen Con Data\n");

    pthread_t* C2S = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(C2S, NULL, &forwarder, C2SA);
    //printf("Start C2S\n");

    forwarder((void*)S2CA);
    //printf("Term\n");

    //Inform the Grim Reaper that we have died
    close(MyCon->ClientCon);
    close(MyCon->ServerCon);
    //printf("Died\n");
}

//The Grim Reaper
void garbageCollector(){
    vector<int> ToDel;

    for (int i = 0; i < Cons.size(); i++){
        if (Cons[i]->Status == 0){
            ToDel.push_back(i);
        }
    }


    for (int i = ToDel.size() - 1; i > -1 ; i--){
        //Cleanup Code Here

        //free(Cons[i]);
        //Threads.erase(Threads.at(iter));
        //Cons.erase(Cons.at(iter));

    }

}

int main(){
    int cserver;
    int ccon;
    int counter = 0;

    while (true){
        cserver = openserver(HostIP,HostPort);
        printf("Awaiting Connection\n");
        ccon = accept(cserver);
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
