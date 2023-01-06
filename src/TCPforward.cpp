#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <signal.h>
#include <math.h>

#include "UNIXstreams.h"
#include "TCPstreams2.h"
#include "MainConfigLoader.h"

using namespace std;

configLoader* MainConfig;

int mainThreadFlag = 1;

struct forwarderArgs {
  struct TCPConnection* From;
  struct TCPConnection* To;
  int* Status;
  int bufsize;
};

void* forwarder(void * MyArgs){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can detect and fix these properly
    struct forwarderArgs* InputArgs = (struct forwarderArgs*)MyArgs;
    char Buffer[InputArgs->bufsize];

    int haveRead;

    while (*InputArgs->Status){
        haveRead = read(InputArgs->From->fd,Buffer,InputArgs->bufsize);
        if (haveRead <= 0){
            *InputArgs->Status = 0;
        }
        else {
            send(InputArgs->To->fd,Buffer,haveRead, 0);
        }
    }
}


class clientHandler {
    public:
        clientHandler(struct TCPConnection* ServerCon,int ID, int BufSize) {
            this->ServerCon = ServerCon;
            this->ClientCon = TCPopenclient(MainConfig->getDestIP(),MainConfig->getDestPort());

            this->ID = ID;
            this->bufsize = BufSize;



            if (this->ClientCon == NULL){
                this->Status = 0;
                this->DeadOnArrival = 1;
                printf("Connection Failed\n");
            }
            else {
                this->Status = 1;
                this->DeadOnArrival = 0;
            }
            

            this->C2SA.bufsize = this->bufsize;
            this->S2CA.bufsize = this->bufsize;
            this->C2SA.Status = &this->Status;
            this->S2CA.Status = &this->Status;

            this->C2SA.From = this->ClientCon;
            this->C2SA.To = this->ServerCon;

            this->S2CA.To = this->ClientCon;
            this->S2CA.From = this->ServerCon;


            
            if (this->Status){
                this->C2ST = (pthread_t*)malloc(sizeof(pthread_t));
                pthread_create(this->C2ST, NULL, &forwarder, &C2SA);

                this->S2CT = (pthread_t*)malloc(sizeof(pthread_t));
                pthread_create(this->S2CT, NULL, &forwarder, &S2CA);

                printf("Client Init %i\n",this->ID);
            }

        }

        int getStatus(){
            return this->Status;
        }

        void setStatus(int Status){
            this->Status = Status;
        }

        ~clientHandler(){
            void* retval;

            this->Status = 0;

            if (this->DeadOnArrival == 0){
                pthread_cancel(*this->C2ST);
                pthread_cancel(*this->S2CT);

                pthread_join(*this->C2ST,&retval);
                pthread_join(*this->S2CT,&retval);

                close(this->ClientCon->fd);
                
                free(this->C2ST);
                free(this->S2CT);
            }
            close(this->ServerCon->fd);

            delete this->ClientCon;
            delete this->ServerCon;

            printf("Client Destroy %i\n",this->ID);
        }

        

    private:
        struct TCPConnection* ServerCon;
        struct TCPConnection* ClientCon;

        int Status;
        int DeadOnArrival;

        int ID;
        int bufsize;

        pthread_t* C2ST;
        pthread_t* S2CT;

        struct forwarderArgs S2CA;
        struct forwarderArgs C2SA;

};

vector<clientHandler*> Clients;

void garbageCollector(){
    //Dead Thread / Connection Indexes are stored in this vector
    vector<int> ToDel;

    printf("GC Start\n");
    for (int i = 0; i < Clients.size(); i++){
        if (Clients[i]->getStatus() == 0){
            ToDel.push_back(i);
        }
    }

    for (int i = ToDel.size() - 1; i > -1 ; i--){
        delete Clients[ToDel[i]];
        Clients.erase(Clients.begin() + ToDel[i]);
    }
}


class controlServer {
    private:
        struct UNIXServer* ControlServerSocket;
        struct UNIXConnection* MyConnection;
        int RUN = 1;
        int CLIENTCON = 1;

        int integerStringSize(int Input){
            return (((int)ceil(log10(Input + 1)))+ 1);
        }

    public:
        controlServer(){
            this->ControlServerSocket = UNIXopenserver((char*)MainConfig->getHostIP());
            if (ControlServerSocket == NULL){
                printf("Failed to Open Control Server\n");
                exit(1);
            }

            this->RUN = 1;
            this->CLIENTCON = 1;
        }

        void run(){
            char* MSG_BUF;
            while (this->RUN){
                this->MyConnection = UNIXaccept(this->ControlServerSocket);
                this->CLIENTCON = 1;
                while (this->CLIENTCON){
                    MSG_BUF = UNIXgetdat(this->MyConnection,1);

                    free(MSG_BUF);
                }
                free(this->MyConnection);
            }
        }

        ~controlServer(){
            free(this->ControlServerSocket);
        }

};

void* controlThreadFunction(void *){
    controlServer* MainControl = new controlServer();
    MainControl->run();
    printf("Control Server Terminate\n");
    delete MainControl;
}

void mainServer(){
    printf("Forwarding %s:%i to %s:%i buffer %i limit is %i\n",MainConfig->getHostIP(),MainConfig->getHostPort(),MainConfig->getDestIP(),MainConfig->getDestPort(),MainConfig->getBufferSize(),MainConfig->getConnectionLimit());

    struct TCPServer* cserver = TCPopenserver(MainConfig->getHostIP(),MainConfig->getHostPort());

    if (cserver == NULL){
        printf("Failed to open Server Socket, Check Permissions!\n");
        exit(1);
    }

    struct TCPConnection* NewServerCon;
    clientHandler* NewClient;

    int IDcounter = 0;
    while (mainThreadFlag){
        if ((Clients.size() > MainConfig->getConnectionLimit()) && (MainConfig->getConnectionLimit() != 0)){
            while (Clients.size() > MainConfig->getConnectionLimit()){
                garbageCollector();
                usleep(1000);
            }
        }

        garbageCollector();

        NewServerCon = TCPaccept(cserver);
        printf("Accept Client %i\n",IDcounter);
        printf("Running %i\n",Clients.size());

        NewClient = new clientHandler(NewServerCon,IDcounter,1024);

        Clients.push_back(NewClient);

        IDcounter ++;
    }






}

int main(int argc, char** argv){
    //Load Configuration
    MainConfig = new configLoader(argc,argv);
    printf("Loaded Configuration\n");


    //The Control Server
    pthread_t* ControlServerThread;
    void* Blank;
    if (MainConfig->getControlFlag() == 1){
        printf("Starting Control Server\n");
        ControlServerThread = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(ControlServerThread, NULL, &controlThreadFunction,Blank);

    }

    //The Main Server
    printf("Start Server\n");
    mainServer();

    //Clean Up
    if (MainConfig->getControlFlag() == 1){
        pthread_cancel(*ControlServerThread);
        pthread_join(*ControlServerThread,&Blank);
        free(ControlServerThread);
        printf("Cleaned Up Control Server\n");
    }

    for (int i = 0; i < Clients.size(); i++){
        Clients[i]->setStatus(0);
    }
    garbageCollector();
    printf("Cleaned Up Connections\n");
}