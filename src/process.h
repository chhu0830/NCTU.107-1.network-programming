#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 16384
#define MAX_PIPE_NUM 4096
#define MAX_PIPE_LATE 1024
#define MAX_FILENAME_LENGTH 1024
#define UNKNOWN_COMMAND_ERRNO -1

struct CMD {
  char cmd[MAX_COMMAND_LENGTH];
  char **argv;
  int argc, fd[2];
};

struct PROCESS {
  struct CMD cmds[MAX_PIPE_NUM];
  int input, output, count, num, error;
  char filename[MAX_FILENAME_LENGTH];
};

void parse_pipe(struct PROCESS *process, char *buf);
void parse_args(struct PROCESS *process);
int build_in(struct CMD *cmd);
void set_io(struct PROCESS *process, int (*fd)[2]);
void exec_cmds(struct PROCESS *process);
void free_process(struct PROCESS *process);
void decrease(int (*fd)[2]);
