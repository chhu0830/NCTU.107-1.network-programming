struct USER;

int create_socket();
int listen_socket(int sockfd, const char *HOST, const int PORT);
struct USER* accept_client(int sockfd, struct USER *users);
void welcome_message(struct USER *user);
