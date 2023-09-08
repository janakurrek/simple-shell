#ifndef __COMMANDS_H
#define COMMANDS_H_

int echo(char *prompt);

int cd(char *prompt);

int pwd(); 

int jobs(int num_jobs, int job_table[]);

int delete_job(int pid, int num_jobs, int job_table[]);

int implement_external_command(char *args[]);

#endif