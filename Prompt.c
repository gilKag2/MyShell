//
// Created by gil on 25/03/19.
//

#include <string.h>
#include "Prompt.h"
#define  INPUT_SIZE 512

void displayPrompt()
{
        printf("> ");
}

void evaluateInput(char* input, char** args)
{
        fgets(input, INPUT_SIZE, stdin);
        char * line = strtok(input, "\n");
        parseToArgs(line, args);
}

void parseToArgs(char* input, char ** args) {

    int counter = 0;
    char * token = strtok(input, " ");
    while(token != NULL) {
        args[counter++] = token;
        token = strtok(NULL, " ");
    }
}


