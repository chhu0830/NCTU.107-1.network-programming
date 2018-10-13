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

    while (command = strsep(&p, "|")) {
      wordexp(command, &exp, 0);
      _argc = exp.we_wordc;
      _argv = exp.we_wordv;

      if (strcmp(_argv[0], "printenv") == 0) print_env(_argc, _argv);
      else if (strcmp(_argv[0], "setenv") == 0) set_env(_argc, _argv);
      else if (strcmp(_argv[0], "exit") == 0) return 0;
    }
  }

  return 0;
}
