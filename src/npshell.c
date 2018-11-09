#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "process.h"
#include "connect.h"
#include "user.h"

void proc_exit()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

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

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setenv("PATH", "bin:.", 1);
    signal(SIGCHLD, proc_exit);

    
    int sockfd = create_socket();
    listen_socket(sockfd, "192.168.1.86", 8080);

    struct USER user;
    while (1) {
        reset_user(&user, 0);
        accept_client(sockfd, &user);

        npshell(&user);

        close(user.sockfd);
    }

    return 0;
}
