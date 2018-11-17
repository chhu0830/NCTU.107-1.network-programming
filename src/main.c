#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include "connect.h"
#include "user.h"
#include "npshell.h"

struct USER *user;

void SIGCHLD_HANDLER()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void SIGINT_HANDLER()
{
    free_users();
    exit(0);
}

void RECV_MSG_HANDLER()
{
    dprintf(user->sockfd, "%s", user->msg);
    user->msg[0] = '\0';
}

void USER_PIPE_HANDLER()
{
    char fifo[32];
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (user->userctl[i] == 1) {
            sprintf(fifo, "/tmp/0756020-%d-%d", i+1, user->id);
            user->userfd[i] = open(fifo, O_RDONLY|O_NONBLOCK|O_CLOEXEC);
            user->userctl[i] = 0;
        } else if (user->userctl[i] == 2) {
            close(user->userfd[i]);
            user->userctl[i] = 0;
        }
    }
}

int main(int argc, const char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGINT, SIGINT_HANDLER);
    signal(SIGCHLD, SIGCHLD_HANDLER);
    signal(SIGUSR1, RECV_MSG_HANDLER);
    signal(SIGUSR2, USER_PIPE_HANDLER);

    const char *host = HOST;
    const int port = (argc == 2) ? atoi(argv[1]) : PORT;
    int sockfd = listen_socket(create_socket(), host, port);
    
    fd_set allset;
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);

    while (1) {
        fd_set rset = allset;
        int nready;

        if ((nready = select(maxfd(sockfd)+1, &rset, NULL, NULL, NULL)) > 0) {
            if (FD_ISSET(sockfd, &rset)) {
                user = accept_client(sockfd);
#if defined(MULTI)
                int pid = 0;
                if ((pid = fork()) < 0) {
                    fprintf(stderr, "Fork error\n");
                } else if (pid > 0) {
                    user->pid = pid;
                    close(user->sockfd);
                    continue;
                }
                FD_CLR(sockfd, &allset);
                close(sockfd);
#endif
                FD_SET(user->sockfd, &allset);
                nready--;
#if defined(SINGLE) || defined(MULTI)
                welcome_msg(user);
                broadcast_msg("*** User '%s' entered from %s/%d. ***\n", user->name, user->ip, user->port);
#endif
                dprintf(user->sockfd, "%% ");
            }

            char buf[MAX_INPUT_LENGTH];
            for (int i = 0; nready && i < MAX_USER_NUM; i++) {
#if defined(SIMPLE) || defined(SINGLE)
                user = &users[i];
#endif
                if (FD_ISSET(user->sockfd, &rset)) {
                    if (read_until_newline(user->sockfd, buf) < 0 || npshell(user, buf) < 0) {
                        FD_CLR(user->sockfd, &allset);
                        leave(user);
#if defined(MULTI)
                        exit(0);
#endif
                    } else {
                        dprintf(user->sockfd, "%% ");
                    }
                    nready--;
                }
            }
        }
    }

    return 0;
}
