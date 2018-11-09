#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "process.h"

void proc_exit()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void npshell()
{
    int numfd[MAX_NUMBERED_PIPE][2] = {};
    char buf[MAX_INPUT_LENGTH];

    while (fputs("% ", stdout), fgets(buf, MAX_INPUT_LENGTH, stdin)) {
        buf[strcspn(buf, "\n\r")] = '\0';
        if (buf[0] == 0) continue;

        struct PROCESS process;
        memset(&process, 0, sizeof(process));

        parse_pipe(&process, buf);
        parse_redirect(&process);
        if (parse_args(&process) < 0) return;

        if (!build_in(&process.cmds[0])) {
            exec_cmds(&process, numfd);
            free_process(&process);
        }
        move_numfd(numfd);
    }
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setenv("PATH", "bin:.", 1);
    signal(SIGCHLD, proc_exit);

    npshell();

    return 0;
}
