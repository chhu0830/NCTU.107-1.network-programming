#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 65536
#define MAX_PIPE_NUM 1024
#define UNKNOWN_COMMAND_ERROR -1

int call(const char *argv[], const char *envp[], int input, int error)
{
  int pfd[2], pid = 0, status;

  if (pipe(pfd) < 0) return -1;
  if ((pid = fork()) < 0) return -1;
  else if (pid == 0) {
    dup2(input, 0);
    dup2(pfd[1], 1);

    if (error) dup2(pfd[1], 2);
    if (execvpe(argv[0], argv, envp) == -1) {
      fprintf(stderr, "Unknown command: [%s].\n", argv[0]);
      exit(UNKNOWN_COMMAND_ERROR);
    }
    exit(0);
  }
  else {
    close(pfd[1]);
    waitpid(pid, &status, 0);
    close(input);
    dup2(pfd[0], input);
    close(pfd[0]);
  }
  return status;
}

void push(int input, int (*np)[2], int num)
{
  int len;
  char *buf[MAX_INPUT_LENGTH];
  if (np[num][0] == 0 && np[num][1] == 0) {
    if (pipe(np[num]) < 0) return -1;
  }
  while (len = read(input, buf, MAX_INPUT_LENGTH)) {
    write(np[num][1], buf, len);
  }
  close(input);
}

int pop(int input, int (*np)[2])
{
  int len;
  char buf[MAX_INPUT_LENGTH];
  for (int i = 1; i < MAX_PIPE_NUM; i++) {
    np[i-1][0] = np[i][0];
    np[i-1][1] = np[i][1];
  }
  if (np[0][0]) {
    close(input);
    dup2(np[0][0], input);
    close(np[0][0]);
    close(np[0][1]);
  }
}



int main(int argc, const char *argv[], const char *envp[])
{
  setbuf(stdout, NULL);
  setenv("PATH", "bin:.", 1);

  char buf[MAX_INPUT_LENGTH], bak[MAX_INPUT_LENGTH];
  char **argv_;
  int argc_;
  wordexp_t exp;
  int np[MAX_PIPE_NUM][2] = {};
  int num, top = 0;

  while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
    buf[strcspn(buf, "\n\r")] = 0;
    if (buf[0] == 0) continue;

    char *p = buf, *command, *filename = NULL, *ptr;
    int input = dup(0), output = dup(1), len, status = -1, num = 0;

    strcpy(bak, buf);
    pop(input, np);

    while (command = strsep(&p, "|!")) {
      if (ptr = strchr(command, '>')) {
        *ptr = 0;
        filename = ptr + 2;
      }
      if (isdigit(command[0]) && sscanf(command, "%d", &num)) {
        push(input, np, num);
        break;
      }

      wordexp(command, &exp, 0);
      argc_ = exp.we_wordc;
      argv_ = exp.we_wordv;

      if (strcmp(argv_[0], "printenv") == 0) {
        if (ptr = getenv(argv_[1])) {
          printf("%s\n", ptr);
        }
      } else if (strcmp(argv_[0], "setenv") == 0) {
        setenv(argv_[1], argv_[2], 1);
      } else if (strcmp(argv_[0], "exit") == 0) {
        return 0;
      } else {
        status = call(argv_, envp, input, p && bak[p-buf-1] == '!');
      }
    }

    if (filename) {
      close(output);
      output = open(filename, O_WRONLY|O_CREAT, 0644);
    }
    while (status == 0 && num == 0 && (len = read(input, buf, MAX_INPUT_LENGTH))) {
      write(output, buf, len);
    }

    close(input);
    close(output);
  }

  return 0;
}
