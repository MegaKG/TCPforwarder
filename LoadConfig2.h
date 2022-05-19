#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstring>
#endif
#ifndef __cplusplus
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

struct data {
	char* Key;
	char* Value;
};

struct configData {
	struct configData* Next;
	struct data MyData;
};

struct configData* readConfig(char* Filename){
	FILE* cf = fopen(Filename,"r");
	fseek(cf,0,SEEK_SET);
	
	struct configData* Head = (struct configData*)malloc(sizeof(struct configData));
	Head->Next = NULL;
	Head->MyData.Key = NULL;
	Head->MyData.Value = NULL;
	
	char KeyBuf[64];
	char ValueBuf[128];
	memset(KeyBuf,0,64);
	memset(ValueBuf,0,128);
	
	int iter = 0;
	char ch;
	char* ptr;
	while (1){
		if (feof(cf)){
			break;
		}
		else {
			ch = fgetc(cf);
			//printf("Ch is %i %c\n",ch,ch);
			
			if (ch == '\n'){
				if ((strlen(KeyBuf) > 0) && (strlen(ValueBuf) > 0)){
					//printf("End Line\n");
					//Copy to Dedicated pointers
					ptr = (char*)malloc(sizeof(char) * (strlen(KeyBuf) + 1));
					memset(ptr,0,strlen(KeyBuf) + 1);
					strcpy(ptr,KeyBuf);
					Head->MyData.Key = ptr;
					//printf("Loaded Key %s -> ",ptr);
					
					ptr = (char*)malloc(sizeof(char) * (strlen(ValueBuf) + 1));
					memset(ptr,0,strlen(ValueBuf) + 1);
					strcpy(ptr,ValueBuf);
					Head->MyData.Value = ptr;
					//printf("%s\n",ptr);
					
					//Generate a new head
					struct configData* NewHead = (struct configData*)malloc(sizeof(struct configData));
					NewHead->Next = Head;
					Head = NewHead;
					Head->MyData.Key = NULL;
					Head->MyData.Value = NULL;
				
				}
				else {
					//printf("Line was Empty\n");
					;
				}
				
				//Clear up for the next line
				iter = 0;
				memset(KeyBuf,0,64);
				memset(ValueBuf,0,128);
				
				
			}
			
			else if (ch == '#'){
				//printf("Comment Line\n");
				
				while (1){
					if (feof(cf)){
						break;
					}
					else {
							ch = fgetc(cf);
							if (ch == '\n'){
								break;
							}
					}
					
				}
				//printf("End Comment Line\n");
				//Delete Line as it is a comment
				//Clear up for the next line
				iter = 0;
				memset(KeyBuf,0,64);
				memset(ValueBuf,0,128);
				
			}
			
			else if ((ch == '\r') || (ch == ' ') || (ch == '\t')){
				//printf("WhiteSpace\n");
				//Do Nothing
				;
			}
			
			else if (ch == '='){
				//printf("Key to Value\n");
				iter = 64;
			}
			
			else {
				//printf("Normal CH");
				if (iter >= 64){
					//printf(" Data\n");
					ValueBuf[iter-64] = ch;
				}
				else {
					//printf(" Key\n");
					KeyBuf[iter] = ch;
				}
				iter += 1;
				
				
			}
			
			
		}
	}
	
	
	fclose(cf);
	//printf("End File\n");
	return Head;
}



void printConfig(struct configData* Config){
	if (Config->Next == NULL){
		printf("Next is NULL, Error\n");
	}
	else {
	
		struct configData* Sel = Config->Next;
		while (1){
			printf("Config: %s -> %s\n",Sel->MyData.Key,Sel->MyData.Value);
			if (Sel->Next == NULL){
				break;
			}
			else {
				Sel = Sel->Next;
			}
		}
	}
	
}


char* getConfValue(const char* Key,struct configData* Config){
	if (Config->Next == NULL){
		printf("Next is NULL, Error\n");
		return NULL;
	}
	
	struct configData* Sel = Config->Next;
	while (1){
		
		if (strcmp(Sel->MyData.Key,Key) == 0){
			return Sel->MyData.Value;
		}
		
		
		if (Sel->Next == NULL){
			break;
		}
		else {
			Sel = Sel->Next;
		}
	}
	return NULL;
	
}

void destroyConfig(struct configData* Config){
	struct configData* ptr;
	int depth;
	while (Config->Next != NULL){
		ptr = Config;
		depth = 0;
		while (1){
			//printf("Depth %i\n",depth);
			if (ptr->Next->Next != NULL){
				ptr = ptr->Next;
			}
			else {
				free(ptr->Next->MyData.Key);
				free(ptr->Next->MyData.Value);
				free(ptr->Next);
				ptr->Next = NULL;
				break;
			}
			depth += 1;
		}
	}
	free(Config->MyData.Key);
	free(Config->MyData.Value);
	free(Config);
}

struct configData* readArgs(int argc, char** argv){
	
	
	struct configData* Head = (struct configData*)malloc(sizeof(struct configData));
	Head->Next = NULL;
	Head->MyData.Key = NULL;
	Head->MyData.Value = NULL;
	
	int KeyPtr = -1;
	int ValuePtr = -1;
	
	char* ptr;
	
	int i;
	for (i = 1; i < argc; i++){
		//printf("arg %s\n",argv[i]);
		if (argv[i][0] =='-'){
				if ((KeyPtr > 0) && (ValuePtr > 0)){
					//Copy to Dedicated pointers
					Head->MyData.Key = argv[KeyPtr];
					//printf("Loaded Key %s -> ",ptr);
					
					
					Head->MyData.Value = argv[ValuePtr];
					//printf("%s\n",ptr);
					
					//Generate a new head
					struct configData* NewHead = (struct configData*)malloc(sizeof(struct configData));
					NewHead->Next = Head;
					Head = NewHead;
					Head->MyData.Key = NULL;
					Head->MyData.Value = NULL;
				
				}
				else {
					//printf("Line was Empty\n");
					;
				}
				
				//Attach new key
				//printf("Key %s\n",argv[i]);
				KeyPtr = i;
		}
		else {
				//printf("Value %s\n",argv[i]);
				ValuePtr = i;
		}
		
		
			
	}
	
	
	//Clean up Residual
	if ((KeyPtr > 0) && (ValuePtr > 0)){
		//Copy to Dedicated pointers
		Head->MyData.Key = argv[KeyPtr];
		//printf("Loaded Key %s -> ",ptr);
					
		Head->MyData.Value = argv[ValuePtr];
		//printf("%s\n",ptr);
					
		//Generate a new head
		struct configData* NewHead = (struct configData*)malloc(sizeof(struct configData));
		NewHead->Next = Head;
		Head = NewHead;
		Head->MyData.Key = NULL;
		Head->MyData.Value = NULL;
	}
	
	


	return Head;
}


void destroyArgsConfig(struct configData* Config){
	if (Config->Next == NULL){
		printf("Next is NULL, Error\n");
		return;
	}
	
	else {
		struct configData* ptr;
		int depth;
		while (Config->Next != NULL){
			ptr = Config;
			depth = 0;
			while (1){
				//printf("Depth %i\n",depth);
				if (ptr->Next->Next != NULL){
					ptr = ptr->Next;
				}
				else {
					free(ptr->Next);
					ptr->Next = NULL;
					break;
				}
				depth += 1;
			}
		}
	}
	free(Config);
}
