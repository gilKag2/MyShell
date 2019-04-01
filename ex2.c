/***************
* Gil Kagan
* 315233221
***********/


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
    pid_t pid;
    char mode;
    int index;
};

//hold info about the shell.
struct ShellInfo {
    struct Job *jobs[MAX_JOBS];
    char cwd[INPUT_SIZE];
    char pwd[INPUT_SIZE];
    int numOfJobs;
};

struct ShellInfo shellInfo;

/*
 * adds the job to the jobs list, and updates its index.
 */
void addJobToList(struct Job *job) {
    shellInfo.jobs[shellInfo.numOfJobs] = NULL;
    job->index = shellInfo.numOfJobs;
    shellInfo.jobs[shellInfo.numOfJobs] = job;
    shellInfo.numOfJobs++;
}

/*
 * updates the parent working directory, by setting it to be the current before the move.
 */
void updatePwd() {
    strcpy(shellInfo.pwd, shellInfo.cwd);
}

/*
 * update current working directory.
 */
void updateCwd() {
    getcwd(shellInfo.cwd, sizeof(shellInfo.cwd));
}

void freeJob(struct Job* job) {
    free(job);
}

/*
 * free the memory allocated for each job.
 */
void freeShell() {
    int i;
    for (i = 0; i < MAX_JOBS; i++) {
        if (shellInfo.jobs[i] != NULL) {
           freeJob(shellInfo.jobs[i]);
        }
    }

}

/*
 * prints each job id and command.
 */
void printJobs() {
    int i;
    for (i = 0; i < shellInfo.numOfJobs; i++) {
        printf("%d %s\n", shellInfo.jobs[i]->pid, shellInfo.jobs[i]->command);
    }
}


/*
 * removes the job from the list.
 */
void removeJob(int idx) {
    shellInfo.jobs[idx] = NULL;
    int i;
    for (i = idx; i < shellInfo.numOfJobs - 1; i++) {
        shellInfo.jobs[i] = shellInfo.jobs[i + 1];
        shellInfo.jobs[i]->index--;
        shellInfo.jobs[i + 1] = NULL;
        freeJob(shellInfo.jobs[i+1]);
    }
    shellInfo.numOfJobs--;

}

/*
 *check if the jobs finished, and if they did remove them.
 */
void checkStatus() {
    int status;
    int i;
    for (i = 0; i < shellInfo.numOfJobs; i++) {
        if (waitpid(shellInfo.jobs[i]->pid, &status, WNOHANG) != 0) {
            removeJob(i);
        }
    }
}

/*
 * activates the "cd" command. deals specifically with '-' and '~' and empty path.
 */
void activateCdCommand(char **args) {
    printf("%d\n", getpid());
    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        if (chdir(getenv("HOME")) != 0) {
            fprintf(stderr, "Error in system call");
            return;
        }
        updatePwd();
        updateCwd();

    } else if (strcmp(args[1], "-") == 0) {
        // if we changed directory at least once. else just skip it.
        if (strcmp(shellInfo.pwd, shellInfo.cwd) != 0) {
            printf("%s\n", shellInfo.pwd);
            if (chdir(shellInfo.pwd) != 0)
                fprintf(stderr, "Error in system call");
            // update the parent directory to be the current.
            updatePwd();
            // update the current directory.
            updateCwd();
        }
    } else {
        char str[INPUT_SIZE] = "";
        strcpy(str, args[1]);
        int i = 2;
        // concats the strings with spaces to act as one string.
        while (args[i] != NULL){
            strcat(str, " ");
            strcat(str, args[i++]);

        }
        if (chdir(str) != 0)
            fprintf(stderr, "Error in system call");
        updatePwd();
        updateCwd();
    }
}


/*
 * executes the "cd", "exit" and "jobs" commands.
 */
int execBuiltIn(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        activateCdCommand(args);
        return 1;
    } else if (strcmp(args[0], "exit") == 0) {
        int i = 0;
        while (args[i] != NULL)
            args[i] = NULL;
        if (args != NULL)
            free(args);
        freeShell();
        return -1;
    } else if (strcmp(args[0], "jobs") == 0) {
        checkStatus();
        printJobs();
        return 1;
    }
    return 0;

}

/*
 * execute process in the child, and also allowes background execution if specified.
 */
int execute(struct Job *job, char **args) {

    pid_t pid;
    int status;
    if ((pid = fork()) == 0) {
        printf("%d\n", getpid());
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "Error in system call");
        }

    } else if (pid != -1) {
        if (job->mode == BACKGROUND) {
            job->pid = pid;
            return pid;
        }
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        fprintf(stderr, "Error in system call");
    }
    return 1;
}

/*
 * get input from user.
 */
void readLine(char *input) {
    fgets(input, INPUT_SIZE, stdin);
    strtok(input, "\n");
}

/*
 * removes any qutation marks from the string.
 */
void removeQutation(char *string) {
  char * name,  *tmp, line[INPUT_SIZE] = "";
  strcpy(line, string);
  tmp = strtok(line, "\"");
  if (tmp == NULL)
      return;
  name = strtok(NULL, "\"");
    if (name == NULL)
        return;
  strcpy(string, tmp);
  strcat(string, name);
}

/*
 * parses the line to arguments.
 */
char **parseToArgs(char *line, char **args) {


   removeQutation(line);
    int counter = 0;
    char *token = strtok(line, " ");

    while (token != NULL) {
        args[counter] = token;
        counter++;
        token = strtok(NULL, " ");
    }
    args[counter] = NULL;
    return args;
}

/*
 * displays the prompt with the current working directory.
 */
void displayPrompt() {
    printf("%s > ", shellInfo.cwd);
}

/*
 * initialize the info.
 */
void init() {
    shellInfo.numOfJobs = 0;
    updateCwd();
    updatePwd();
}

/*
 * activates the shell by looping and executing the given commands.
 */
void activateShell() {
    int status = 1;
    // one array for the input line, and the other is the parsed array.

    do {
        char line[INPUT_SIZE];
        char **args = (char **) malloc(INPUT_SIZE * sizeof(char *));;
        // in case of alloc error.
        if (!args) {
            fprintf(stderr, "Error in system call");
            continue;
        }

        displayPrompt();
        readLine(line);
        char mode = FOREGROUND;
        // empty input.
        if (strlen(line) < 2) continue;

        if (line[strlen(line) - 1] == '&') {
            mode = BACKGROUND;
            line[strlen(line) - 1] = '\0';
        }
        struct Job *job = (struct Job *) malloc(sizeof(struct Job));
        if (!job) {
            fprintf(stderr, "Error in system call");
            continue;
        }
        strcpy(job->command, line);
        job->pid = -1;
        job->mode = mode;
        parseToArgs(line, args);
        addJobToList(job);
        int res = 0;
        if ((res = execBuiltIn(args)) == 0)
            status = execute(job, args);
        if (res == -1) return;
        int i=0;
        while (args[i] != NULL)
            args[i] = NULL;
        if (args != NULL)
            free(args);

        usleep(500000);
    } while (status);
    exit(0);
}

int main() {
    init();
    activateShell();
    return 0;
}