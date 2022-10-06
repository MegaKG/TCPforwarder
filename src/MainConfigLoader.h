#include "LoadConfig2.h"
#include <stdio.h>
#include <stdlib.h>
#include "StringInputs2.h"
#include <string>
#include <string.h>
#include <iostream>
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


//This function loads the Configuration
int loadSettings(int argc, char** argv, string *HostIP, string *DestIP, int *HostPort, int *DestPort, int *bufsize, string *ControlSocketPath, int *ControlSocket, int *ConLimit ){
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
        DestIP->assign(getConfValue("Client_address",MyArgs));
        *HostPort = stosi(getConfValue("Host_port",MyArgs));
        *DestPort = stosi(getConfValue("Client_port",MyArgs));
        

       
        //Optional Arguments
        if (getConfValue("Host_address",MyArgs) != NULL){
            HostIP->assign(getConfValue("Host_address",MyArgs));
        }
        if (getConfValue("Buffer_size",MyArgs) != NULL){
            *bufsize = stosi(getConfValue("Buffer_size",MyArgs));
        }
        if (getConfValue("Connection_limit",MyArgs) != NULL){
            *ConLimit = stosi(getConfValue("Connection_limit",MyArgs));
        }
        if (getConfValue("Control_socket",MyArgs) != NULL){
            ControlSocketPath->assign(getConfValue("Control_socket",MyArgs));
            *ControlSocket = 1;
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
        DestIP->assign(getConfValue("-dest",MyArgs));
        *HostPort = stosi(getConfValue("-hport",MyArgs));
        *DestPort = stosi(getConfValue("-dport",MyArgs));


        //Optional Arguments
        if (getConfValue("-host",MyArgs) != NULL){
            HostIP->assign(getConfValue("-host",MyArgs));
        }

        if (getConfValue("-buf",MyArgs) != NULL){
            *bufsize = stosi(getConfValue("-buf",MyArgs));
        }
        if (getConfValue("-climit",MyArgs) != NULL){
            *ConLimit = stosi(getConfValue("-climit",MyArgs));
        }
        if (getConfValue("-control",MyArgs) != NULL){
            ControlSocketPath->assign(getConfValue("-control",MyArgs));
            *ControlSocket = 1;
        }

        destroyArgsConfig(MyArgs);

        return 0;

    }


    
}