#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

const char* BadVal = "127.0.0.1";

char* getIP(char* Hostname){
    struct hostent *hstnm;
    hstnm = gethostbyname(Hostname);

    char* ReturnVal = (char*)malloc(sizeof(char) * 16);
    memset(ReturnVal,0,16);
    if (hstnm == NULL){
        strcpy(ReturnVal,BadVal);
        return ReturnVal;
    }

    //Return only the first address
    sprintf(ReturnVal,"%i.%i.%i.%i",(unsigned char)hstnm->h_addr_list[0][0],(unsigned char)hstnm->h_addr_list[0][1],(unsigned char)hstnm->h_addr_list[0][2],(unsigned char)hstnm->h_addr_list[0][3]);
    return ReturnVal;
}
