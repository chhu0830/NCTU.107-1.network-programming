#define HOST "0.0.0.0"
#define PORT 8000

int create_socket();
int listen_socket(int sockfd, const char *host, const int port);
struct USER* accept_client(int sockfd);
int maxfd(int sockfd);
