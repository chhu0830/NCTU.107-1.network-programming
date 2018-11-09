#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "process.h"
#include "user.h"
#include "npshell.h"

void npshell(struct USER *user)
{
    int numfd[MAX_NUMBERED_PIPE][2] = {}, len;
    char buf[MAX_INPUT_LENGTH];

    while (write(user->sockfd, "% ", 2) && (len = read(user->sockfd, buf, MAX_INPUT_LENGTH)) > 0)
    {
        buf[strcspn(buf, "\n\r")] = '\0';
        if (buf[0] == 0) continue;

        struct PROCESS process;
        memset(&process, 0, sizeof(struct PROCESS));

        parse_pipe(&process, buf);
        parse_redirect(&process);
        if (parse_args(&process) < 0) return;

        set_io(&process, numfd, user->sockfd);
        if (exec(&process) < 0) {
            return;
        }
        move_numfd(numfd);
    }
}
