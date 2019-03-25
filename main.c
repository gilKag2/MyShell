#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include "Prompt.h"

int main() {


    while (1) {
        int status;
        __pid_t  pid;
        if ((pid = fork()) == 0) {
            wait(&status);
        } else{
            execv("enter command and path here, wait for child to finish. remove this command from the jobs"
                  " and if next command is exit break loop")
        }

        if () {
            fprintf(stderr, "Error in system call!");
            break;
        }
    }
    return 0;
}