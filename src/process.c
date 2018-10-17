#include "process.h"

void parse_pipe(struct PROCESS *process, char *buf)
{
  int i;
  char *cmd;
  for(i = 0; (cmd = strsep(&buf, "|")); i++) {
    strcpy(process->cmds[i].cmd, cmd);
  }
  process->count = i;
}

void parse_args(struct PROCESS *process)
{
  char *ptr, *cmd = process->cmds[process->count - 1].cmd;

  if ((ptr = strchr(cmd, '>'))) {
    *ptr = '\0';
    sscanf(ptr+1, "%s", process->filename);
  } else if ((ptr = strchr(cmd, '!'))) {
    *ptr = '\0';
    sscanf(ptr+1, "%d", &process->num);
    process->error = 1;
  } else if (process->count != 1 && isdigit(cmd[0])) {
    sscanf(cmd, "%d", &process->num);
    process->count -= 1;
  }

  for (int i = 0; i < process->count; i++) {
    wordexp_t exp;
    cmd = process->cmds[i].cmd;
    wordexp(cmd, &exp, 0);

    process->cmds[i].argc = exp.we_wordc;
    process->cmds[i].argv = calloc(sizeof(char*), (exp.we_wordc+1));

    for (unsigned int j = 0; j < exp.we_wordc; j++) {
      process->cmds[i].argv[j] = strdup(exp.we_wordv[j]);
    }
    wordfree(&exp);
  }
}


int build_in(struct CMD *cmd)
{
  char *ptr;
  if (strcmp(cmd->argv[0], "exit") == 0) {
    exit(0);
  } else if (strcmp(cmd->argv[0], "setenv") == 0) {
    setenv(cmd->argv[1], cmd->argv[2], 1);
  } else if (strcmp(cmd->argv[0], "printenv") == 0) {
    if ((ptr = getenv(cmd->argv[1]))) {
      printf("%s\n", ptr);
    }
  } else {
    return 0;
  }
  return 1;
}

void set_io(struct PROCESS *process, int (*fd)[2])
{
  if (fd[0][0]) {
    process->input = fd[0][0];
    close(fd[0][1]);
  } else {
    process->input = dup(0);
  }

  if (process->filename[0]) {
    process->output = open(process->filename, O_WRONLY|O_TRUNC|O_CREAT, 0644);
  } else if (process->num) {
    if (fd[process->num][1] == 0) {
      while (pipe(fd[process->num]) < 0);
    }
    process->output = dup(fd[process->num][1]);
  } else {
    process->output = dup(1);
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
      if (i == 0) {
        dup2(process->input, 0);
      } else {
        dup2(process->cmds[i-1].fd[0], 0);
      }
      if (i == process->count - 1) {
        dup2(process->output, 1);
        if (process->error) {
          dup2(process->output, 2);
        }
      } else {
        dup2(process->cmds[i].fd[1], 1);
      }

      write(fd[1], "0", 1);
      if (execvp(process->cmds[i].argv[0], process->cmds[i].argv) == -1) {
        fprintf(stderr, "Unknown command: [%s].\n", process->cmds[i].argv[0]);
        exit(UNKNOWN_COMMAND_ERRNO);
      }
      exit(0);
    } else {
      process->cmds[i].pid = pid;
      while (read(fd[0], buf, 1) == 0);
      if (i == 0) close(process->input);
      else close(process->cmds[i-1].fd[0]);
      if (i == process->count - 1) close(process->output), close(process->cmds[i].fd[0]);
      close(process->cmds[i].fd[1]);
    }
  } 
  if (process->num == 0) {
    for (int i = 0, status; i < process->count; i++) {
      waitpid(process->cmds[i].pid, &status, 0);
    }
  }
  close(fd[0]);
  close(fd[1]);
}

void free_process(struct PROCESS *process)
{
  for (int i = 0; i < process->count; i++) {
    for (int j = 0; j < process->cmds[i].argc; j++) {
      free(process->cmds[i].argv[j]);
    }
    free(process->cmds[i].argv);
  }
}

void decrease(int (*fd)[2])
{
  for (int i = 1; i < MAX_PIPE_LATE; i++) {
    fd[i-1][0] = fd[i][0];
    fd[i-1][1] = fd[i][1];
  }
}
