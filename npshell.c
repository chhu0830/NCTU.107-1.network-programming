#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 256

void print_env(int argc, const char *argv[])
{
  if (argc < 2) {
    fputs("Arg num error!!\n", stderr);
    return;
  }
  printf("%s\n", getenv(argv[1]));
}

void set_env(int argc, const char *argv[])
{
  if (argc < 3) {
    fputs("Arg num error!!\n", stderr);
    return;
  }
  setenv(argv[1], argv[2], 1);
}

int call(const char *argv[], const char *envp[], int input)
{
  int pfd[2], pid = 0, status;

  if (pipe(pfd) < 0) return -1;
  if ((pid = fork()) < 0) return -1;
  else if (pid == 0) {
    close(pfd[0]);
    dup2(input, 0);
    dup2(pfd[1], 1);
    execvpe(argv[0], argv, envp);
    exit(0);
  }
  else {
    close(pfd[1]);
    wait(&status);
    close(input);
    dup2(pfd[0], input);
  }
  return status;
}


int main(int argc, const char *argv[], const char *envp[])
{
  setbuf(stdout, NULL);
  setenv("PATH", "bin:.", 1);

  char buf[MAX_INPUT_LENGTH], *command;
  char **_argv;
  int _argc;
  wordexp_t exp;

  while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
    buf[strcspn(buf, "\n")] = 0;
    if (buf[0] == 0) continue;
    char *p = buf;
    int input = dup(0), len, flag = -1;

    while (command = strsep(&p, "|")) {
      wordexp(command, &exp, 0);
      _argc = exp.we_wordc;
      _argv = exp.we_wordv;

      if (strcmp(_argv[0], "printenv") == 0) print_env(_argc, _argv);
      else if (strcmp(_argv[0], "setenv") == 0) set_env(_argc, _argv);
      else if (strcmp(_argv[0], "exit") == 0) return 0;
      else flag = call(_argv, envp, input);
    }

    while(flag == 0 && (len = read(input, buf, MAX_INPUT_LENGTH))) {
      buf[len] = 0;
      fputs(buf, stdout);
    }
    close(input);
  }

  return 0;
}
