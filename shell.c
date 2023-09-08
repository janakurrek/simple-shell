// Libraries
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Built-in Commands
#include "commands.h"
#include "piping.h"

// Global Variables
int FOREGROUND_PID;
int LENGTH = 20;
int JOB_TABLE[25]; /* array to store the current list of running jobs */
int NUM_JOBS = 0; /* number of jobs that are running */

// // // // // // // // // //
//     Signal Handling     //
// // // // // // // // // //

// Ctrl + C
void handle_sigint(int signal) {
    /* terminate the foreground application */
    kill(FOREGROUND_PID, SIGTERM);
    return;
}

void handle_sigchld(int signal) {
    /* remove terminated children from the job table */
    int DEAD_PID;
    while ((DEAD_PID=waitpid(-1, NULL, WNOHANG)) > 0) {
        delete_job(DEAD_PID, NUM_JOBS, JOB_TABLE);
        NUM_JOBS -=1;
    }
    return;
}

// // // // // // // // // //
//     Get User Commands   //
// // // // // // // // // //

int getcmd(char *prompt, char *args[], int *background, int *redirection, int *piping) {
    /* parse the user command in char *args[] */
    int length, flag, i = 0;
    char *token;
    char *loc1, *loc2, *loc3;
    char *line = NULL;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    /* check for empty command */
    if (length == 0) {
        exit(-1);
    }

    /* handle Ctrl + D */
    else if (length == -1){
         exit(0);
    }

    /* check background specification */
    if ((loc1 = index(line, '&')) != NULL) {
        *background = 1;
        *loc1 = ' ';
    } else {
        *background = 0;
    }
    
    /* check output redirection */
    if ((loc2 = index(line, '>')) != NULL) {
        *redirection = 1;
    } else {
        *redirection = 0;
    }

    /* check piping */
    if ((loc3 = index(line, '|')) != NULL) {
        *piping = 1;
    } else {
        *piping = 0;
    }

    /* clear arguments */
    memset(args, '\0', LENGTH);

    /* split the command and store in args */
    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++) {
            /* first 32 ascii characters are non-printable */
            if (token[j] <= 32) {
                token[j] = '\0'; 
            }
        }
        if (strlen(token) > 0) {
            args[i++] = token;
        }
    }
    /* return the number of entries in the array */
    return i;
}

// // // // // // // // // //
//        Main Method      //
// // // // // // // // // //

int main(void) {
    // Flags
    char* args[LENGTH]; /* array to store the current list of commands */
    int redirection; /* flag for output redirection */
    int piping; /* flag for piping */
    int bg; /* flag for running processes in the background */
    int cnt; /* count of arguments in the command */

    // Foreground PID
    FOREGROUND_PID = getpid();

    // Ctrl + C
    if (signal(SIGINT, handle_sigint) == SIG_ERR){ 
        printf("ERROR: Could not bind signal handler for SIGINT\n");
        exit(1);
    }
    
    // Ctrl + Z
    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR){ 
        printf("ERROR: Could not bind signal handler for SIGTSTP\n");
        exit(1);
    }

    // SIGCHLD
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR){ 
        printf("ERROR: Could not bind signal handler for SIGINT\n");
        exit(1);
    }

    // Terminated Children
    
    while(1){
        /* reset flags for background processing, output redirection, and piping */
        bg = 0;    
        redirection = 0;
        piping = 0;

        // // // // // // // // // // // //
        //     Parse the User Commands   //
        // // // // // // // // // // // //
        
        /* number of arguments submitted by the user */
        cnt = getcmd("\n>> ", args, &bg, &redirection, &piping);
        
        /* user has not inputted a command */
        if ((cnt == 0)||(strcmp(args[0], "") == 0)) {
            printf("Warning: Input at least one command.\n");
            continue;
        }

        /* the user is running more than 25 jobs */
        if (NUM_JOBS >= 25) {
            printf("Error: Capacity exceeded. Please wait for at least one background process to terminate.\n");
            continue;
        }

        // // // // // // // // // // // //
        //       Built-In Functions      //
        // // // // // // // // // // // //

        int CHILD_PID = fork();

        if (CHILD_PID == 0) {
            // child process
            int status_code;
            if (piping == 1) {
                /* initialize auxiliary arrays */
                char* args1[cnt];
                char* args2[cnt];
                /* identify the index of the pipe symbol */
                int pipe_idx;
                for (int i = 0; i < cnt; i ++) {
                    if (strcmp(args[i], "|") == 0) {
                        pipe_idx = i;
                    }
                }
                /* populate the auxiliary arrays */
                int k1 = 0, k2 = 0;
                for(int i = 0; i < cnt; i++) {  
                    if(i < pipe_idx)  
                        args1[k1++] = args[i];
                    else if (i > pipe_idx) {
                        args2[k2++] = args[i];
                    } else {
                        continue;
                    }
                }
                /* append NULL to each auxiliary array */
                args1[k1++] = (char*) NULL;
                args2[k2++] = (char*) NULL;
                /* run piped commands */
                status_code = run_pipe(args1, args2);
            } else if (redirection == 1) {
                continue;
            } else {
                /* determine status of external command */
                status_code = implement_external_command(args);
                /* take appropriate action */
                if (status_code < 0) {
                    printf("Invalid command. Please try again.\n");
                    exit(1); 
                } else {
                    exit(0);
                }
            }
        } else if (CHILD_PID == -1){      
            // FORK FAILURE
            printf("ERROR: fork failed\n");
            exit(1);
        } else {
            // PARENT PROCESS
            if (bg == 0) {
                /* set the foreground pid to the child pid */
                FOREGROUND_PID = CHILD_PID;
                /* do not run the child in the background */
                waitpid(CHILD_PID, NULL, 0);
            } else {
                /* set the foreground pid to the parent pid */
                FOREGROUND_PID = getpid();
                /* update the background job table */
                JOB_TABLE[NUM_JOBS] = CHILD_PID;
                NUM_JOBS += 1;
            }
            /* execute built-in commands */
            if (strcmp(args[0], "exit") == 0){
                /* terminate shell successfully */
                exit(0);
            } else if (strcmp(args[0], "echo") == 0){
                /* run built-in implementation of echo */
                for(int i = 1; i < cnt; i++){
                    echo(args[i]);
                }
            } else if (strcmp(args[0], "cd") == 0){
                /* run built-in implementation of cd */
                cd(args[1]);
            } else if (strcmp(args[0], "pwd") == 0){
                /* run built-in implementation of pwd */
                pwd();
            } else if (strcmp(args[0], "fg") == 0){
                /* run built-in implementation of fg */
                int position = atoi(args[1]);
                if ((position > NUM_JOBS) || (position < 0)) {
                    printf("Invalid position. Please try again.");
                } else {
                    /* fetch the pid for the background process */
                    int bg_pid = JOB_TABLE[position];
                    printf("Process %d is running in the foreground...\n", bg_pid);
                    /* wait for the job to terminate in the foreground*/
                    waitpid(bg_pid, NULL, 0);
                    delete_job(bg_pid, NUM_JOBS, JOB_TABLE);
                    NUM_JOBS -=1;
                }
            } else if (strcmp(args[0], "jobs") == 0){
                /* print the jobs table */
                jobs(NUM_JOBS, JOB_TABLE);
            }
        }
    }
}