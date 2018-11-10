#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "connect.h"
#include "user.h"
#include "npshell.h"
#include "process.h"

#define HOST "192.168.1.86"
#define PORT 8080

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setenv("PATH", "bin:.", 1);
    signal(SIGCHLD, SIGCHLD_HANDLER);

    int sockfd = create_socket();
    listen_socket(sockfd, HOST, PORT);

    struct USER users[MAX_USER_NUM], *user;
    reset_users(users);

    while (1) {
        user = available_user(users);
        accept_client(sockfd, user);

        npshell(user);

        reset_user(user);
    }

    return 0;
}
