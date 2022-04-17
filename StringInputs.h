#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

unsigned int stoui(char* input){
    int index = 0;
    unsigned int output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    unsigned int units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

unsigned long stoul(char* input){
    int index = 0;
    unsigned long output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    unsigned long units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

unsigned long long stoull(char* input){
    int index = 0;
    unsigned long long output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    unsigned long long units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

uint8_t stoui8(char* input){
    int index = 0;
    uint8_t output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    uint8_t units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

uint16_t stoui16(char* input){
    int index = 0;
    uint16_t output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    uint16_t units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

uint32_t stoui32(char* input){
    int index = 0;
    uint32_t output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    uint32_t units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

uint64_t stoui64(char* input){
    int index = 0;
    uint64_t output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '\x00'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards
    uint64_t units = 1;
    char ch;
    for (int i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    return output;
}

double stod(char* input){
    int index = 0;
    double output = 0;

    //Locate the End of the String
    while (1){
        if (input[index] == '.'){
            index -= 1;
            break;
        }
        else {
            index += 1;
        }
    }

    //Work Backwards for Left of Dot
    double units = 1;
    char ch;
    int i;
    for (i = index; i > -1; i--){
        ch = input[i];
        //printf("%c %i\n",ch,ch);
        if ((ch < 48) || (ch > 57)){
            return 0;
        }
        output += units * (ch-48);
        units = units * 10;
    }
    

    //Work Forwards for Right of Dot
    units = 0.1;
    i = index+2;
    while (1){
        ch = input[i];
        if (ch == '\x00'){
            break;
        }
        else {
            //printf("%c %i\n",ch,ch);
            if ((ch < 48) || (ch > 57)){
                return 0;
            }
            output += units * (ch-48);
            units = units/10;
            i += 1;
        }
        
    }
    return output;
}