#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 256
#define MAX_PIPE_NUM 256
#define UNKNOWN_COMMAND_ERROR -1

struct NUM_PIPE {
  int fd, count;
};

int call(const char *argv[], const char *envp[], int input, int error)
{
  int pfd[2], pid = 0, status;

  if (pipe(pfd) < 0) return -1;
  if ((pid = fork()) < 0) return -1;
  else if (pid == 0) {
    close(pfd[0]);
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
    wait(&status);
    close(input);
    dup2(pfd[0], input);
    close(pfd[0]);
  }
  return status;
}

void push(int input, struct NUM_PIPE *np, int top, int num)
{
  int i;
  for (i = top - 1; i >= 0; i--) {
    if (num >= np[i].count) np[i + 1] = np[i];
    else break;
  }
  np[i + 1].count = num;
  np[i + 1].fd = dup(input);
}

int pop(int input, struct NUM_PIPE *np, int top)
{
  int i, pfd[2], flag = 1, len;
  char buf[MAX_INPUT_LENGTH];
  for (i = 0; i < top; np[i++].count--);
  while (top && np[top-1].count == 0) {
    if (flag) {
      if (pipe(pfd) < 0) return -1;
      close(input);
      dup2(pfd[0], input);
      close(pfd[0]);
      flag = 0;
    }
    while (len = read(np[top-1].fd, buf, MAX_INPUT_LENGTH)) {
      write(pfd[1], buf, len);
    }
    close(np[--top].fd);
  }
  if (flag == 0) close(pfd[1]);
  return top;
}



int main(int argc, const char *argv[], const char *envp[])
{
  setbuf(stdout, NULL);
  setenv("PATH", "bin:.", 1);

  char buf[MAX_INPUT_LENGTH], bak[MAX_INPUT_LENGTH];
  char **_argv;
  int _argc;
  wordexp_t exp;
  struct NUM_PIPE np[MAX_PIPE_NUM];
  int num, top = 0;

  while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
    buf[strcspn(buf, "\n\r")] = 0;
    if (buf[0] == 0) continue;
    strcpy(bak, buf);

    char *p = buf, *command, *filename = NULL, *ptr;
    int input = dup(0), output = dup(1), len, status = -1, num = 0;

    top = pop(input, np, top);
    while (command = strsep(&p, "|!")) {
      if (ptr = strchr(command, '>')) *ptr = 0, filename = ptr + 2;
      if (isdigit(command[0]) && sscanf(command, "%d", &num)) {
        push(input, np, top++, num);
        break;
      }

      wordexp(command, &exp, 0);
      _argc = exp.we_wordc;
      _argv = exp.we_wordv;

      if (strcmp(_argv[0], "printenv") == 0) printf("%s\n", getenv(_argv[1]));
      else if (strcmp(_argv[0], "setenv") == 0) setenv(_argv[1], _argv[2], 1);
      else if (strcmp(_argv[0], "exit") == 0) return 0;
      else status = call(_argv, envp, input, p && bak[p-buf-1] == '!');
    }

    if (filename) close(output), output = open(filename, O_WRONLY|O_CREAT, 0644);
    while (status == 0 && num == 0 && (len = read(input, buf, MAX_INPUT_LENGTH))) {
      write(output, buf, len);
    }

    close(input);
    close(output);
  }

  return 0;
}
