#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h>

// // // // // // // // // // // //
//             PIPING            //
// // // // // // // // // // // //

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

int run_pipe(char *args1[], char *args2[]){
    /* initialize pipe variables */
    int fd[2];
    int pid;

    /* start pipe */
    if(pipe(fd) == -1) {
        printf("Error: Piping failed. Please try again.\n");
        return -1;
    }
    pid = fork();
    if (pid == 0) {
        close(fd[READ_END]);
        dup2(fd[WRITE_END], WRITE_END);
        close(fd[WRITE_END]);
        execvp(args1[0], args1);
        exit(0);
    } else if (pid < 0) {
        printf("Error: Piping failed. Please try again.\n");
        return -1;
    } else {
        close(fd[WRITE_END]);
        dup2(fd[READ_END], READ_END);
        close(fd[READ_END]);
        execvp(args2[0], args2);
        wait(NULL);
        exit(0);
    }
    return 0;
}
