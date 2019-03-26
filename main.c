#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include "Prompt.h"
#define  INPUT_SIZE 512
#define  NUM_OF_ARGS 512
int main() {

    // one array for the input line, and the other is the parsed array.
    char input[INPUT_SIZE], *args[NUM_OF_ARGS];
    while (1) {
        displayPrompt();
        evaluateInput(input, args);
        int status;
        __pid_t  pid;
        if ((pid = fork()) == 0) {
            wait(&status);
        } else{
            printf("New process ID: %d\n", pid);

            execv("enter command and path here, wait for child to finish. remove this command from the jobs"
                  " and if next command is exit break loop")
        }

        if (strcmp(args[0], 'g') == 0) {
            fprintf(stderr, "Error in system call!");
            break;
        }
    }
    return 0;
}