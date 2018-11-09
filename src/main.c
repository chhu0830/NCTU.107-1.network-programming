#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "npshell.h"
#include "connect.h"
#include "user.h"

void proc_exit()
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
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
