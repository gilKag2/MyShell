#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#define  INPUT_SIZE 512
#define  NUM_OF_ARGS 512
#define  MAX_JOBS 512
#define BACKGROUND 0
#define FOREGROUND 1

struct Job {
    char command[INPUT_SIZE];
    char * args[INPUT_SIZE];
    pid_t  pid;
    char mode;
    int index;
};

struct ShellInfo {
    struct Job* jobs[MAX_JOBS];
    char cwd[INPUT_SIZE];
    char pwd[INPUT_SIZE];
    int numOfJobs;
} shellInfo;


void addJobToList(struct Job* job) {
    job->index = shellInfo.numOfJobs;
    shellInfo.jobs[shellInfo.numOfJobs] = job;
    shellInfo.numOfJobs++;
}

int execProcess(struct Job* job) {
    pid_t  pid;
    int status;
    if ((pid = fork()) == 0) {
        printf("%d\n", getpid());
        if (execvp(job.args[0], job.args) == -1) {
          fprintf(stderr, "Error in system call");
        }

    } else if (pid != -1){
        if (job->mode == BACKGROUND) {
            job->pid = pid;
            printf("%d\n", pid);
            // add the job to the jobs list.
            addJobToList(job);
            return pid;
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


void updatePwd(){
    strcpy(shellInfo.pwd, shellInfo.cwd);
}

void updateCwd() {
    getcwd(shellInfo.cwd, sizeof(shellInfo.cwd));
}

void activateCdCommand(char** args) {
    printf("%d\n", getpid());
    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        if (chdir(getenv("HOME")) != 0)
            fprintf(stderr, "Error in system call");
    } else if (strcmp(args[1], "-")== 0) {
        if (shellInfo.pwd[0] != '\0') {
            printf("%s\n", shellInfo.pwd);

            if (chdir(shellInfo.pwd) != 0)
                fprintf(stderr, "Error in system call");
            // update the parent directory to be the current.
            updatePwd();
            // update the current directory.
            updateCwd();
        }
        else
            fprintf(stderr, "Error in system call");
    }
    else{
        if (chdir(args[1]) != 0)
            fprintf(stderr, "Error in system call");
    }
}

void freeShell() {
    free(shellInfo.cwd);
    free(shellInfo.pwd);
    int i;
    for (i = 0; i < shellInfo.numOfJobs; i++) {
        shellInfo.jobs[i] = NULL;
    }
}

void printJobs() {
    int i;
    for (i = 0; i < shellInfo.numOfJobs; i++) {
        printf("%s\n", shellInfo.jobs[i]->command);
    }
}
int execute(char ** args) {
    if (args[0] == NULL) {
        // no command
        return 1;
    }
    if (strcmp(args[0], "cd") == 0) {
        activateCdCommand(args);
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0) {

        exit(0);
    } else if(strcmp(args[0], "jobs") == 0) {
        printJobs();
    }

    return execProcess();
}


void readLine(char* input) {
    fgets(input, INPUT_SIZE, stdin);
    strtok(input, "\n");
}

void parseToArgs(char* line, char** args) {
    int counter = 0;
    char * token = strtok(line, " ");
    while(token != NULL) {
        args[counter++] = token;
        token = strtok(NULL, " ");
    }
}

void displayPrompt() {
    printf("< ");
}

void init() {
    int i;
    for (i = 0; i < MAX_JOBS; i++) {
        shellInfo.jobs[i] = NULL;
    }
    shellInfo.numOfJobs = 0;
}

void startShell() {
    int status;
    // one array for the input line, and the other is the parsed array.
    char line[INPUT_SIZE], *args[NUM_OF_ARGS];

    do{
        displayPrompt();
        readLine(line);
        int isBackground = 0;
        if (line[strlen(line) - 1] == '&') {
            isBackground = 1;
            line[strlen(line) - 1] = '\0';
        }
        parseToArgs(line, args);

        free(line);
        free(args);
    } while (status);
}

int main() {
    startShell();
    return 0;
}