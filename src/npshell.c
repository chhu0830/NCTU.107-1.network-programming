#include "process.h"

void proc_exit()
{
  int status;
  while (waitpid(-1, &status, WNOHANG) > 0);
}

void npshell(char *buf, int (*fd)[2])
{
  struct PROCESS process;
  memset(&process, 0, sizeof(process));

  parse_pipe(&process, buf);
  parse_args(&process);

  if (!build_in(&process.cmds[0])) {
    exec_cmds(&process, fd);
    free_process(&process);
  }
  decrease(fd);
}

int main()
{
  setvbuf(stdout, NULL, _IONBF, 0);
  setenv("PATH", "bin:.", 1);
  signal(SIGCHLD, proc_exit);

  int fd[MAX_PIPE_LATE][2] = {};
  char buf[MAX_INPUT_LENGTH];

  while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
    buf[strcspn(buf, "\n\r")] = '\0';
    if (buf[0] == 0) continue;

    npshell(buf, fd);
  }

  return 0;
}
