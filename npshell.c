#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 65536
#define MAX_PIPE_NUM 1024
#define MAX_PIPE_LATE 1024
#define MAX_FILENAME_LENGTH 1024
#define UNKNOWN_COMMAND_ERROR -1

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


void proc_exit(int signal)
{
  int status;
  wait(&status);
}

void parse_pipe(struct PROCESS *process, char *buf)
{
  int i;
  char *cmd;
  for(i = 0; cmd = strsep(&buf, "|"); i++) {
    strcpy(process->cmds[i].cmd, cmd);
  }
  process->count = i;
}

void parse_args(struct PROCESS *process)
{
  char *ptr, *cmd = process->cmds[process->count - 1].cmd;

  if (ptr = strchr(cmd, '>')) {
    *ptr = '\0';
    strcpy(process->filename, ptr+2);
    process->output = open(process->filename, O_WRONLY|O_TRUNC|O_CREAT, 0644);
  } else if (ptr = strchr(cmd, '!')) {
    *ptr = '\0';
    sscanf(ptr+1, "%d", &process->num);
    process->error = 1;
  } else if (process->count != 1 && isdigit(cmd[0])) {
    sscanf(cmd, "%d", &process->num);
    process->count -= 1;
  }

  wordexp_t exp;
  for (int i = 0; i < process->count; i++) {
    cmd = process->cmds[i].cmd;
    wordexp(cmd, &exp, 0);

    process->cmds[i].argc = exp.we_wordc;
    process->cmds[i].argv = malloc(sizeof(char*) * exp.we_wordc);

    for (int j = 0; j < exp.we_wordc; j++) {
      process->cmds[i].argv[j] = strdup(exp.we_wordv[j]);
    }
  }
}

void exec_cmds(struct PROCESS *process)
{
  int pid;
  int fd[2];
  char buf[8];
  while (pipe(fd) < 0);
  for (int i = 0; i < process->count; i++) {
    while (pipe(process->cmds[i].fd) < 0);
    while ((pid = fork()) < 0);
    if (pid == 0) {
      if (i == 0) dup2(process->input, 0);
      else dup2(process->cmds[i-1].fd[0], 0);
      if (i == process->count - 1) dup2(process->output, 1);
      else dup2(process->cmds[i].fd[1], 1);

      write(fd[1], "0", 1);
      if (execvp(process->cmds[i].argv[0], process->cmds[i].argv) == -1) {
        fprintf(stderr, "Unknown command: [%s].\n", process->cmds[i].argv[0]);
        exit(UNKNOWN_COMMAND_ERROR);
      }
      exit(0);
    } else {
      while (read(fd[0], buf, 1) == 0);
      if (i == 0) close(process->input);
      else close(process->cmds[i-1].fd[0]);
      if (i == process->count - 1) close(process->output), close(process->cmds[i].fd[0]);
      close(process->cmds[i].fd[1]);
    }
  } 
  close(fd[0]);
  close(fd[1]);
}

int main(int argc, const char *argv[])
{
  setbuf(stdout, NULL);
  setenv("PATH", "bin:.", 1);
  signal(SIGCHLD, proc_exit);

  char buf[MAX_INPUT_LENGTH];

  while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
    buf[strcspn(buf, "\n\r")] = 0;
    if (buf[0] == 0) continue;

    char filename[MAX_FILENAME_LENGTH];
    struct PROCESS process;
    memset(&process, 0, sizeof(process));
    process.input = dup(0);
    process.output = dup(1);

    parse_pipe(&process, buf);
    parse_args(&process);
    exec_cmds(&process);

  }

  return 0;
}
