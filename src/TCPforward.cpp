#include <iostream>
#include "TCPstreams2.hpp"
#include "StringInputs2.h"
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <cstdlib>
#include <signal.h>
#include "LoadConfig2.h"

using namespace std;




//Filler Initial Values, to be initialised
string HostIP = "127.0.0.1";
string DestIP = "127.0.0.1";
int HostPort = 5000;
int DestPort = 5000;
int bufsize = 1024;
int ConLimit = 0;


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

    //Check for Bad Connection
    if (NewCon != NULL){
        //Extract FD
        MyCon->ClientCon = NewCon->fd;
        //Here we deallocate the connection struct pointer as we have already extracted the file descriptor
        free(NewCon);
        
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
        //Here we deallocate the connection struct pointer as we have already extracted the file descriptor
        free(NewCon);
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

//Prints the Help Text
void printHelp(char** argv){
    printf("Usage:\n%s\n",argv[0]);
    printf("\t-h\t: This Help Menu\n");
    printf("\t-c\t: Configuration File, Specified by -c FilePath. EG: -c Example.conf\n");
    printf("\t-host\t: The IP Address to host the TCP server on. Usually 0.0.0.0 (Optional)\n");
    printf("\t-dest\t: The IP Address Of the destination.\n");
    printf("\t-hport\t: The Hosting Port\n");
    printf("\t-dport\t: The Destination Port\n");
    printf("\t-buf\t: The Buffer size, usually 1024 (Optional)\n");
	printf("\t-climit\t: The Connection Limit, Set to 0 for unlimited. Default is 0 (Optional)\n");
}



//Required Configuration Values
const int confValues = 3;

//For the File
const char* ConfTags[3] {
    "Client_address",
    "Client_port",
    "Host_port",

};

//For the Cmdline Arguments
const char* ArgTags[3]{
    "-dest",
    "-hport",
    "-dport",
};

//This function loads the Configuration
int loadSettings(int argc, char** argv){
    struct configData* MyArgs = readArgs(argc,argv);
    //printf("Got the Following Arguments:\n");
    //printConfig(MyArgs);
    if (getConfValue("-c",MyArgs) != NULL){
        char* CF_Name = getConfValue("-c",MyArgs);
        printf("Config File Specified, %s\n",CF_Name);

        //DeAllocate Args
        destroyArgsConfig(MyArgs);

        //Load the Config into the same struct pointer
        MyArgs = readConfig(CF_Name);

        //Check Flags
        for (int i = 0; i < confValues; i++){
            //printf("Loading %s\n",ConfTags[i]);
            if (getConfValue(ConfTags[i],MyArgs) == NULL){
                printf("%s is not defined\n",ConfTags[i]);
                destroyConfig(MyArgs);
                return -1;
            }
        }

        //Assign Values
        DestIP.assign(getConfValue("Client_address",MyArgs));
        HostPort = stosi(getConfValue("Host_port",MyArgs));
        DestPort = stosi(getConfValue("Client_port",MyArgs));
        

       
        //Optional Arguments
        if (getConfValue("Host_address",MyArgs) != NULL){
            HostIP.assign(getConfValue("Host_address",MyArgs));
        }

        if (getConfValue("Buffer_size",MyArgs) != NULL){
            bufsize = stosi(getConfValue("Buffer_size",MyArgs));
        }
        if (getConfValue("Connection_limit",MyArgs) != NULL){
            ConLimit = stosi(getConfValue("Connection_limit",MyArgs));
        }

        
        destroyConfig(MyArgs);

        return 1;

    }

    //Help Command Specified
    else if (getConfValue("-h",MyArgs) != NULL){
        destroyArgsConfig(MyArgs);
        return -2;
    }
    
    
    //Loads CMDline Arguments
    else {
        //Check Flags
        for (int i = 0; i < confValues; i++){
            //printf("Loading %s\n",ArgTags[i]);
            if (getConfValue(ArgTags[i],MyArgs) == NULL){
                printf("%s Required Argument not Provided!\n",ArgTags[i]);
                destroyArgsConfig(MyArgs);
                return -1;
            }
        }


        //Assign Arguments            
        DestIP.assign(getConfValue("-dest",MyArgs));
        HostPort = stosi(getConfValue("-hport",MyArgs));
        DestPort = stosi(getConfValue("-dport",MyArgs));


        //Optional Arguments
        if (getConfValue("-host",MyArgs) != NULL){
            HostIP.assign(getConfValue("-host",MyArgs));
        }

        if (getConfValue("-buf",MyArgs) != NULL){
            bufsize = stosi(getConfValue("-buf",MyArgs));
        }
        if (getConfValue("-climit",MyArgs) != NULL){
            ConLimit = stosi(getConfValue("-climit",MyArgs));
        }

        destroyArgsConfig(MyArgs);

        return 0;

    }


    
}

int main(int argc, char** argv){
    signal(SIGPIPE, SIG_IGN); // Ignore Read Errors, the program can fix these properly
    struct ServerSocketData* cserver;
    struct TCPConnection* ccon;
    int counter = 0;

    if (argc == 1){
        printHelp(argv);
        return -1; 
    }

    int loadResult = loadSettings(argc,argv);
    if (loadResult < 0){
        printHelp(argv);
        return -1;
    }

    

    printf("Forwarding %s:%i to %s:%i buffer %i limit is %i\n",HostIP.c_str(),HostPort,DestIP.c_str(),DestPort,bufsize,ConLimit);
    cserver = openserver(HostIP,HostPort);
    while (true){
		
		if ((Cons.size() > ConLimit) && (ConLimit != 0)){
			printf("Exceeded Connection Limit, waiting...\n");
			while (Cons.size() > ConLimit){
				sleep(1);
				garbageCollector();
			}
			printf("Space Free, Proceeding\n");
		}
		
		
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
