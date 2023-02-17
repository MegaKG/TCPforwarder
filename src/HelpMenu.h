#include <stdio.h>
#include <stdlib.h>

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
	printf("\t-control\t: The Control interface, argument is required, but isn't used for now\n");
}