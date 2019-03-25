//
// Created by gil on 25/03/19.
//

#include "Prompt.h"


void displayPrompt()
{
        printf(">");
}

void readInput() {
        char input[512];
        fgets(input, 512, stdin);
        return input;
}


void loop() {

        int status;
        do {
             displayPrompt();


        }while (status);

}