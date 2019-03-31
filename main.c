#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>

#define  INPUT_SIZE 512
#define  MAX_JOBS 512
#define BACKGROUND 0
#define FOREGROUND 1

struct Job {
    char command[INPUT_SIZE];
    pid_t  pid;
    char mode;
    int index;
};

struct ShellInfo {
    struct Job* jobs[MAX_JOBS];
    char cwd[INPUT_SIZE];
    char pwd[INPUT_SIZE];
    int numOfJobs;
};

struct ShellInfo shellInfo;


void addJobToList(struct Job* job) {
    shellInfo.jobs[shellInfo.numOfJobs] = NULL;
    job->index = shellInfo.numOfJobs;
    shellInfo.jobs[shellInfo.numOfJobs] = job;
    shellInfo.numOfJobs++;
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
        struct passwd *pwd = getpwuid(getuid());
        if (chdir(pwd->pw_dir) != 0) {
            fprintf(stderr, "Error in system call");
            return;
        }
        updatePwd();
        updateCwd();

    } else if (strcmp(args[1], "-")== 0) {
        if (strcmp(shellInfo.pwd, shellInfo.cwd) != 0) {
            printf("%s\n", shellInfo.pwd);
            if (chdir(shellInfo.pwd) != 0)
                fprintf(stderr, "Error in system call");
            // update the parent directory to be the current.
            updatePwd();
            // update the current directory.
            updateCwd();
        }
    }
    else{
        char *str;
        strcpy(str, args[1]);
        if (chdir(args[1]) != 0)
            fprintf(stderr, "Error in system call");
    }
}



void freeShell() {
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


void removeJob(int idx)
{
    shellInfo.jobs[idx] = NULL;
    for (int i = idx; i < shellInfo.numOfJobs - 1; i++) {
        shellInfo.jobs[i] = shellInfo.jobs[i+1];
        shellInfo.jobs[i]->index--;
        shellInfo.jobs[i+1] = NULL;
    }
    shellInfo.numOfJobs--;

}
void checkStatus() {
    int status;

    for (int i = 0; i < shellInfo.numOfJobs; i++) {
        if (waitpid(shellInfo.jobs[i]->pid, &status, WNOHANG) != 0) {
            removeJob(i);
        }
    }
}
int execute(struct Job* job, char** args) {
   
    if (strcmp(args[0], "cd") == 0) {
        activateCdCommand(args);
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0) {
        freeShell();
        exit(0);
    } else if(strcmp(args[0], "jobs") == 0) {
        checkStatus();
        printJobs();
        return 1;
    }

    pid_t  pid;
    int status;
    if ((pid = fork()) == 0) {
        printf("%d\n", getpid());
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "Error in system call");
        }

    } else if (pid != -1){
        if (job->mode == BACKGROUND) {
            job->pid = pid;
            printf("%d\n", getpid());
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


void readLine(char* input) {
    fgets(input, INPUT_SIZE, stdin);
    strtok(input, "\n");
}

char** parseToArgs(char* line) {
    char** args = (char**)malloc(INPUT_SIZE * sizeof(char*));
    // in case of alloc error.
    if (!args) {
        fprintf(stderr, "Error in system call");
    }
    int counter = 0;
    char * token = strtok(line, " ");

    while(token != NULL) {
       args[counter] = token;
       counter++;
        token = strtok(NULL, " ");
    }
    return args;
}

void displayPrompt() {
    printf("%s > ", shellInfo.cwd);
}

void init() {
    shellInfo.numOfJobs = 0;
    updateCwd();
    updatePwd();
}


void activateShell() {
    int status;
    // one array for the input line, and the other is the parsed array.

    do{

        char line[INPUT_SIZE];
        char ** args;

        displayPrompt();
        readLine(line);
        char mode = FOREGROUND;
        if (strlen(line) == 0) continue;
        
        if (line[strlen(line) - 1] == '&') {
            mode = BACKGROUND;
            line[strlen(line) - 1] = '\0';
        }
        args = parseToArgs(line);
        struct Job * job = (struct Job*) malloc(sizeof(struct Job));
        strcpy(job->command , line);
        job->pid = -1;
        job->mode = mode;

        status = execute(job, args);
        free(args);
        sleep(1);
    } while (status);

}

int main() {
    init();
    activateShell();
    return 0;
}