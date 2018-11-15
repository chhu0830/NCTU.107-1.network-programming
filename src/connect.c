#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "connect.h"
#include "user.h"


int create_socket()
{
    int sockfd = 0, optval;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket error\n");
        return -1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return sockfd;
}

int listen_socket(int sockfd, const char *HOST, const int PORT)
{
    struct sockaddr_in server_info;
    bzero(&server_info, sizeof(server_info));

    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr(HOST);
    server_info.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sockfd, (const struct sockaddr*)&server_info, sizeof(server_info)) < 0) {
        fprintf(stderr, "Bind error\n");
        return -1;
    }
    
    if (listen(sockfd, MAX_USER_NUM) < 0) {
        fprintf(stderr, "Listen error\n");
        return -1;
    }

    return sockfd;
}

struct USER* accept_client(int sockfd, struct USER *users)
{
    struct sockaddr_in client_info;
    unsigned int len = sizeof(client_info);
    int clientfd = accept(sockfd, (struct sockaddr*)&client_info, &len);

    if (clientfd < 0) {
        fprintf(stderr, "Accept error\n");
        return NULL;
    }
    
    struct USER *user = available_user(users);
    // strcpy(user->ip, inet_ntoa(client_info.sin_addr));
    // user->port = (int)ntohs(client_info.sin_port);
    strcpy(user->ip, "CGILAB");
    user->port = 511;
    user->sockfd = clientfd;

    printf("id: %d, ip: %s, port: %d, sockfd: %d\n", user->id, user->ip, user->port, user->sockfd);

    return user;
}

void welcome_message(struct USER *user)
{
    dprintf(user->sockfd, "****************************************\n");
    dprintf(user->sockfd, "** Welcome to the information server. **\n");
    dprintf(user->sockfd, "****************************************\n");
}
