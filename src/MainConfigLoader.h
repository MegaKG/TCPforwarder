#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>

#include "HelpMenu.h"
#include "StringInputs2.h"
#include "LoadConfig2.h"
#include "dnsresolve.h"

using namespace std;

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

class configLoader {
    private:
        //Initialise with some defaults
        string HostIP = "127.0.0.1";
        string DestIP = "127.0.0.1";
        int HostPort = 5000;
        int DestPort = 5000;
        int bufsize = 1024;

        string ControlSocketPath = "./sock";
        int ControlSocket = 0;

        int ConLimit = 0;

        char DestDNSBuffer[16];
        char HostDNSBuffer[16];

    public:
        const char* getHostIP(){ 
            memset(HostDNSBuffer,0,16);
            char* IP = getIP((char*)this->HostIP.c_str());
            strcpy(HostDNSBuffer,IP);
            free(IP);

            printf("Host IP %s\n",HostDNSBuffer);
            return HostDNSBuffer;

        }
        int getHostPort(){
            return this->HostPort;
        }
        int getDestPort(){
            return this->DestPort;
        }
        int getBufferSize(){
            return this->bufsize;
        }
        int getConnectionLimit(){
            return this->ConLimit;
        }
        const char* getDestIP(){
            memset(DestDNSBuffer,0,16);
            char* IP = getIP((char*)this->DestIP.c_str());
            strcpy(DestDNSBuffer,IP);
            free(IP);

            printf("Dest IP %s\n",DestDNSBuffer);
            return DestDNSBuffer;
        }

        configLoader(int argc, char** argv){
            if (argc == 1){
                printHelp(argv);
                exit(-1);
            }

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
                        printf("Warning %s is not defined, using defaults\n",ConfTags[i]);
                        destroyConfig(MyArgs);
                    }
                }

                //Assign Values
                if (getConfValue("Client_address",MyArgs) != NULL){
                    this->DestIP.assign(getConfValue("Client_address",MyArgs));
                }
                if (getConfValue("Host_port",MyArgs) != NULL){
                    this->HostPort = stosi(getConfValue("Host_port",MyArgs));
                }
                if (getConfValue("Client_port",MyArgs) != NULL){
                    this->DestPort = stosi(getConfValue("Client_port",MyArgs));
                }
                if (getConfValue("Host_address",MyArgs) != NULL){
                    this->HostIP.assign(getConfValue("Host_address",MyArgs));
                }
                if (getConfValue("Buffer_size",MyArgs) != NULL){
                    this->bufsize = stosi(getConfValue("Buffer_size",MyArgs));
                }
                if (getConfValue("Connection_limit",MyArgs) != NULL){
                    this->ConLimit = stosi(getConfValue("Connection_limit",MyArgs));
                }
                if (getConfValue("Control_socket",MyArgs) != NULL){
                    this->ControlSocketPath.assign(getConfValue("Control_socket",MyArgs));
                    this->ControlSocket = 1;
                }

                destroyConfig(MyArgs);

            }

            //Help Command Specified
            else if (getConfValue("-h",MyArgs) != NULL){
                destroyArgsConfig(MyArgs);
                printHelp(argv);
                exit(0);
            }
            
            
            //Loads CMDline Arguments
            else {
                //Check Flags
                for (int i = 0; i < confValues; i++){
                    //printf("Loading %s\n",ArgTags[i]);
                    if (getConfValue(ArgTags[i],MyArgs) == NULL){
                        printf("Warning %s is not defined, using defaults\n",ArgTags[i]);
                        destroyArgsConfig(MyArgs);
                        exit(0);
                    }
                }


                //Assign Arguments   
                if (getConfValue("-dest",MyArgs) != NULL){         
                    this->DestIP.assign(getConfValue("-dest",MyArgs));
                }
                if (getConfValue("-hport",MyArgs) != NULL){   
                    this->HostPort = stosi(getConfValue("-hport",MyArgs));
                }
                if (getConfValue("-dport",MyArgs) != NULL){   
                    this->DestPort = stosi(getConfValue("-dport",MyArgs));
                }


               
                if (getConfValue("-host",MyArgs) != NULL){
                    this->HostIP.assign(getConfValue("-host",MyArgs));
                }

                if (getConfValue("-buf",MyArgs) != NULL){
                    this->bufsize = stosi(getConfValue("-buf",MyArgs));
                }
                if (getConfValue("-climit",MyArgs) != NULL){
                    this->ConLimit = stosi(getConfValue("-climit",MyArgs));
                }
                if (getConfValue("-control",MyArgs) != NULL){
                    this->ControlSocketPath.assign(getConfValue("-control",MyArgs));
                    this->ControlSocket = 1;
                }

                destroyArgsConfig(MyArgs);



            }
        }

};
 