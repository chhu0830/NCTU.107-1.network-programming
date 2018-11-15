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

struct USER *users, *user;

void SIGCHLD_HANDLER()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void RECV_MSG_HANDLER()
{
    dprintf(user->sockfd, "%s\n", user->msg);
}

void RECV_FIFO_HANDLER()
{
    char fifo[32];
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (user->fifo[i] < 0) {
            sprintf(fifo, "/tmp/0756020-%d-%d", i+1, user->id);
            user->fifo[i] = open(fifo, O_RDONLY|O_CLOEXEC);
        }
    }
}

int max(int sockfd)
{
    int maxfd = sockfd;
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd > maxfd) {
            maxfd = users[i].sockfd;
        }
    }
    return maxfd;
}

int main(int argc, const char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setenv("PATH", "bin:.", 1);
    signal(SIGCHLD, SIGCHLD_HANDLER);
    signal(SIGUSR1, RECV_MSG_HANDLER);
    signal(SIGUSR2, RECV_FIFO_HANDLER);

    const char *HOST = "0.0.0.0";
    const int PORT = (argc == 2) ? atoi(argv[1]) : 8000;
    int sockfd = listen_socket(create_socket(), HOST, PORT);
    users = init_users();
    
    fd_set allset;
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);

    char buf[MAX_INPUT_LENGTH];

    while (1) {
        fd_set rset = allset;
        int maxfd = max(sockfd), nready;
        if ((nready = select(maxfd+1, &rset, NULL, NULL, NULL)) > 0) {
            if (FD_ISSET(sockfd, &rset)) {
                user = accept_client(sockfd, users);
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
#endif
                FD_SET(user->sockfd, &allset);
                nready--;
#if defined(SINGLE) || defined(MULTI)
                sprintf(buf, "*** User '%s' entered from %s/%d. ***", user->name, user->ip, user->port);
                welcome_message(user);
                broadcast_msg(users, buf);
#endif
                dprintf(user->sockfd, "%% ");
            }

            for (int i = 0; nready && i < MAX_USER_NUM; i++) {
#if defined(SINGLE)
                user = &users[i];
#endif
                if (FD_ISSET(user->sockfd, &rset)) {
                    if (read_until_newline(user->sockfd, buf) < 0 || npshell(users, user, buf) < 0) {
                        FD_CLR(user->sockfd, &allset);
                        leave(users, user);
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

    free_users(users);
    return 0;
}
