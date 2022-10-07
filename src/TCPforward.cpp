#include <stdio.h>
#include <stdlib.h>
#include "TCPstreams2.h"
#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>

#include "MainConfigLoader.h"

using namespace std;

configLoader* MainConfig;

struct forwarderArgs {
  struct TCPConnection* From;
  struct TCPConnection* To;
  int* Status;
  int bufsize;
};

void* forwarder(void * MyArgs){
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
                printf("Connection Failed\n");
            }
            this->Status = 1;

            this->C2SA.bufsize = this->bufsize;
            this->S2CA.bufsize = this->bufsize;
            this->C2SA.Status = &this->Status;
            this->S2CA.Status = &this->Status;

            this->C2SA.From = this->ClientCon;
            this->C2SA.To = this->ServerCon;

            this->S2CA.To = this->ClientCon;
            this->S2CA.From = this->ServerCon;


            this->C2ST = (pthread_t*)malloc(sizeof(pthread_t));
            pthread_create(this->C2ST, NULL, &forwarder, &C2SA);

            this->S2CT = (pthread_t*)malloc(sizeof(pthread_t));
            pthread_create(this->S2CT, NULL, &forwarder, &S2CA);

            printf("Client Init %i\n",this->ID);

        }

        int getStatus(){
            return this->Status;
        }

        ~clientHandler(){
            void* retval;

            this->Status = 0;

            pthread_cancel(*this->C2ST);
            pthread_cancel(*this->S2CT);

            pthread_join(*this->C2ST,&retval);
            pthread_join(*this->S2CT,&retval);

            //close(this->ClientCon->fd);
            //close(this->ServerCon->fd);

            free(this->C2ST);
            free(this->S2CT);

            delete this->ClientCon;
            delete this->ServerCon;

            printf("Client Destroy %i\n",this->ID);
        }

        

    private:
        struct TCPConnection* ServerCon;
        struct TCPConnection* ClientCon;

        int Status;

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
    while (1){
        if ((Clients.size() > MainConfig->getConnectionLimit()) && (MainConfig->getConnectionLimit() != 0)){
            while (Clients.size() > MainConfig->getConnectionLimit()){
                garbageCollector();
                usleep(1000);
            }
        }

        garbageCollector();

        NewServerCon = TCPaccept(cserver);
        printf("Accept Client %i\n",IDcounter);

        NewClient = new clientHandler(NewServerCon,IDcounter,1024);

        Clients.push_back(NewClient);

        IDcounter ++;
    }






}

int main(int argc, char** argv){
    //Load Configuration
    MainConfig = new configLoader(argc,argv);
    
    printf("Loaded Configuration\n");
    mainServer();
}