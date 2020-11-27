#define _GNU_SOURCE
#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 16384
#define MAX_PIPE_NUM 4096
#define MAX_NUMBERED_PIPE 1024
#define MAX_FILENAME_LENGTH 1024
#define UNKNOWN_COMMAND_ERRNO -1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>


struct CMD {
    char cmd[MAX_COMMAND_LENGTH];
    char **argv;
    int argc, pid;
};

struct PROCESS {
    struct CMD cmds[MAX_PIPE_NUM];
    int input, output, count, num, error;
    char filename[MAX_FILENAME_LENGTH];
};

void parse_pipe(struct PROCESS*, char*);
void parse_redirect(struct PROCESS*);
int parse_args(struct PROCESS*);
int build_in(struct CMD*);
void set_io(struct PROCESS*, int (*)[2]);
void exec_cmds(struct PROCESS*, int (*)[2]);
void move_numfd(int (*)[2]);
void free_process(struct PROCESS*);
