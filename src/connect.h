#define MAX_CONNECT_NUM 30

struct USER;

int create_socket();
int listen_socket(int sockfd, const char *host, int port);
int accept_client(int sockfd, struct USER *user);
void welcome_message(int sockfd);
