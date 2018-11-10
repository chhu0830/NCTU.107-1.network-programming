#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
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

    struct USER user;

    while (1) {
        reset_user(&user, 0);
        accept_client(sockfd, &user);

        npshell(&user);

        close(user.sockfd);
    }

    return 0;
}
