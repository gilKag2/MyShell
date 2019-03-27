#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include "Prompt.h"
#define  INPUT_SIZE 512
#define  NUM_OF_ARGS 512


typedef struct Jobs jobs;
struct jobs {
    char command[INPUT_SIZE];
    pid_t  pid;
};



int activateShell(int isBackground, char** args) {
    pid_t  pid;
    int status;
    if ((pid = fork()) == 0) {
        printf("%d\n", getpid());

        if (execv(args[0], args) == -1) {
          fprintf(stderr, "Error in system call");
        }

    } else if (pid != -1){
        if (isBackground) {
            printf("%d\n", pid);
            return 1;
        }
        do{
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    else {
        fprintf(stderr, "Error in system call");
    }
    return 1;
}




void activateCdCommand(char** args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME")) != 0)
            fprintf(stderr, "Error in system call");
    } else{
        if (chdir(args[1]) != 0)
            fprintf(stderr, "Error in system call");
    }
}


int execute(int isBackground, char ** args) {
    if (args[0] == NULL) {
        // no command
        return 1;
    }
    if (strcmp(args[0], "cd") == 0) {
        activateCdCommand(args);
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0) {
        return 0;
    }
    return activateShell(isBackground, args);
}

int main() {

    int status;
    // one array for the input line, and the other is the parsed array.
    char input[INPUT_SIZE], *args[NUM_OF_ARGS];
    struct Jobs* jobs;
    do{
        displayPrompt();
        evaluateInput(input, args);
        int isBackground = 0;
        if (input[strlen(input) - 1] == '&')
            isBackground = 1;
        status = execute(isBackground, args);
        free(input);
        free(args);
    } while (status);
    return 0;
}