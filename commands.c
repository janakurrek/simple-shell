#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// // // // // // // // // //
//    Built-In Functions   //
// // // // // // // // // //

int echo(char *prompt) {
    /* display the prompt */
    int status_code = printf("%s", prompt);
    /* return the status code for echo */
    if (status_code < 0) {
        printf("Error: The echo command was unsuccessful.\n");
        return -1;
    } else {
        return 1;
    }
}

int cd(char *prompt) {
    /* change the current directory */
    int status_code = chdir(prompt);
    /* return the status code for chdir */
    if (status_code < 0) {
        printf("Error: Changing the directory was unsuccessful.\n");
        return -1;
    } else {
        return 1;
    }
}

int pwd() {
    /* get the working directory */
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
       printf("Error: Printing the current directory was unsuccessful.\n");
       return -1;
    } else {
        /* print the working directory, returning the status code on a failure */
        printf("%s\n", cwd);
        return 1;
    }
}

int jobs(int num_jobs, int job_table[]) {
    if (num_jobs == 0) {
        printf("Remark: No background jobs are running. You can enter one using the '&' symbol.\n");
    } else {
        for (int i = 0; i < num_jobs; i++){  
                printf ("| [%d] | %d |\n", i, job_table[i]);
        }
    }
    return 1;
}

int delete_job(int pid, int num_jobs, int job_table[]) {
    int position = 0;
    for (int i = 0; i < num_jobs; i++) {
        if (job_table[i] == pid) {
            break;
        } else {
            position +=1;
        }
    }
    for (int i = position; i < num_jobs; i++) {
        job_table[i] = job_table[i+1];
    }
    return 1;
}

int implement_external_command(char *args[]) {
    int status_code = 1;
    if ((strcmp(args[0], "echo") != 0) && (strcmp(args[0], "cd") != 0) && (strcmp(args[0], "exit") != 0) &&
        (strcmp(args[0], "pwd") != 0) && (strcmp(args[0], "jobs") != 0) && (strcmp(args[0], "fg") != 0)) {
                status_code = execvp(args[0], args);
    } return status_code;
}