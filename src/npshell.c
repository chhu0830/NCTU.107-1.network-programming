#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "process.h"
#include "user.h"
#include "npshell.h"
#include <stdio.h>

void SIGCHLD_HANDLER()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

int read_until_newline(int fd, char *buf)
{
    int len = 0;
    while (read(fd, buf+len, 1) > 0) {
        len++;
        if (buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            break;
        }
    }

    return --len;
}

void npshell(struct USER *user)
{
    int numfd[MAX_NUMBERED_PIPE][2] = {}, len, status;
    char buf[MAX_INPUT_LENGTH];

    while (write(user->sockfd, "% ", 2) && (len = read_until_newline(user->sockfd, buf)) >= 0)
    {
        if (len == 0) continue;

        struct PROCESS process;
        memset(&process, 0, sizeof(struct PROCESS));

        parse_pipe(&process, buf);
        parse_redirect(&process);
        if (parse_args(&process) < 0) return;

        set_io(&process, numfd, user->sockfd);
        status = exec(&process);
        free_process(&process);

        if (status < 0) {
            return;
        }

        move_numfd(numfd);
    }
}
