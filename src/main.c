#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include "connect.h"
#include "user.h"
#include "npshell.h"

// extern USER user
struct USER *users;

void SIGCHLD_HANDLER()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

int main(int argc, const char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setenv("PATH", "bin:.", 1);
    signal(SIGCHLD, SIGCHLD_HANDLER);

    const int PORT = (argc == 2) ? atoi(argv[1]) : 8000;
    int sockfd = create_socket();
    listen_socket(sockfd, "0.0.0.0", PORT);

    users = init_users();

    while (1) {
        struct USER *user = available_user(users);
        accept_client(sockfd, user);

        int numfd[MAX_NUMBERED_PIPE][2] = {}, len;
        char buf[MAX_INPUT_LENGTH];

#if defined(MULTI)
        int pid = 0;
        if ((pid = fork()) < 0) {
            fprintf(stderr, "Fork error\n");
        } else if (pid > 0) {
            close(user->sockfd);
            continue;
        }
#endif
        while (write(user->sockfd, "% ", 2) && (len = read_until_newline(user->sockfd, buf)) >= 0)
        {
            if (len == 0) continue;
            if (npshell(user, buf, numfd) < 0) break;
        }
        reset_user(user);

#if defined(MULTI)
        exit(0);
#endif
    }

    free_users(users);

    return 0;
}
