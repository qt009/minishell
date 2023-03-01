#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include <sys/stat.h>//     // open()
#include <signal.h>         // signal()
#include <fcntl.h>  //      // open()
#include <stdio.h>          // printf(), ...
#include <time.h>   //          // time(), ..
#include <string.h>

#define MAXLENGTH 100
#define MAXARGS 10

unsigned long timer = 0;
void zombie_handler(int signum)
{
    waitpid ( -1 , NULL , WNOHANG );
}
void read_command(char* input, char** command, char* parameters[]){ //char** command because of core dump problem
    char* get_command = fgets(input, MAXLENGTH, stdin);
    if(get_command == NULL) {
        exit(1);
    }
    size_t endln = strlen(input) - 1;
    if(input[endln] == '\n') {
        input[endln] = '\0';
    }

    *command = strtok(input, " ");
    parameters[0] = *command;
    for(int i = 1; i < MAXARGS + 1; i++) {
        parameters[i] = strtok(NULL, " ");
    }
}

void cd(char* directory) {
    if(directory == NULL) {
        fprintf(stderr,"Directory name not given\n");
        return;
    }

    if(chdir(directory) == -1) {
        fprintf(stderr, "Unable to go to directory %s\n", directory);
    }
}



void exec(char* command, char* parameters[]) {
    int run_in_background = 0;
    for(int i = MAXARGS; i >= 0; i--) {
        if(parameters[i] != NULL) {
            if(strcmp(parameters[i], "&") == 0) {
                run_in_background = 1;
                parameters[i] = NULL;
            }
            break;
        }
    }

    pid_t pid = fork();
    if(pid == -1) {
        fprintf(stderr, "Unable to fork\n");
        exit(1);
    }

    if(pid == 0) {
        execvp(command, parameters);
    } else if(pid > 0) {
        if(run_in_background){
            printf("id:[%d]\n", pid);
            return;
        }
        signal(SIGCHLD,zombie_handler);
        while(waitpid(pid, NULL, 0) == -1); // without this line it'll look like: 'sth'> --> functional but seems troubling
    }
}

void handler(){
    timer = time(0) - timer;
    //double time_elapsed= ((double)begin)/CLOCKS_PER_SEC;
    printf("\nTime elapsed: %ld seconds\n", timer);
    exit(0);
}
int main(){
    timer = time(0);
    char input[MAXLENGTH];
    char* command;
    char* parameters[MAXARGS + 2];

    parameters[MAXARGS + 1] = NULL;

    while(1) {
        signal(SIGINT,handler);
        char dir[MAXLENGTH];

        if (getcwd(dir, MAXLENGTH) == NULL) {
            printf("[directory unavailable!]  >");
        } else {
            printf("%s  >", dir);
        }

        read_command(input, &command, parameters);
        if(command == NULL) {
            continue;
        }

        if(strcmp(command, "exit") == 0) {
            exit(0);
        } else if(strcmp(command, "cd") == 0) {
            cd(parameters[1]);
        } else {
            exec(command, parameters);
        }

    }
    exit(1);
}
